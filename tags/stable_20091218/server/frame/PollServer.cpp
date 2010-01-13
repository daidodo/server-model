#include <common/Logger.h>
#include <common/IterAdapter.h>
#include "PollServer.h"

NS_SERVER_BEGIN

CPollServer::CPollServer(CMainServer & mainServer)
    : CThreadPool(1,mainServer.epollServerStackSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , removeFdQue_(mainServer.removeFdQue_)
    , eventFdQue_(mainServer.eventFdQue_)
    , EVENT_QUE_SZ_(mainServer.tcpServerThreadCount_)
    , stats_(mainServer.serverStatsOn_ ? new __Stats : 0)
{}

CPollServer::~CPollServer()
{
    if(stats_)
        delete stats_;
}

void CPollServer::Init(const __Config & config)
{
    poll_.SetFdTimeout(config.GetInt("server.tcp.fd.timeout",0));  //seconds
    poll_.SetPollTimeout(config.GetInt("poll.epoll.timeout",500)); //ms
    maxFdSz_ = config.GetInt("max.fd.count",1024,64);
}

void CPollServer::Reconfig(const __Config & config)
{
    poll_.SetFdTimeout(config.GetInt("server.tcp.fd.timeout",0));  //seconds
    poll_.SetPollTimeout(config.GetInt("poll.epoll.timeout",500)); //ms
}

void CPollServer::ShowConfig(std::ofstream & file) const
{
    using namespace std;
    if(!file.is_open())
        return;
    file<<"\nPollServer :\n"
        <<"  fdTimeout_ = "<<poll_.GetFdTimeout()<<endl
        <<"  pollTimeout_ = "<<poll_.GetPollTimeout()<<endl
        <<"  maxFdSz_ = "<<maxFdSz_<<endl
        ;
}

int CPollServer::doIt()
{
    LOCAL_LOGGER(logger,"CPollServer::doIt");
    ASSERT(eventFdQue_ && EVENT_QUE_SZ_ > 0,"eventFdQue_="<<eventFdQue_
        <<", EVENT_QUE_SZ_="<<EVENT_QUE_SZ_<<" invalid");
    prepare();
    for(U32 curtime = 0;;){
        if(!poll_.Wait()){
            WARN("poll error"<<CSocket::ErrMsg());
            continue;
        }
        SCOPE_LOGGER(logger,"CPollServer::doIt");
        if(poll_.CanTimeout())
            curtime = time(0);
        U32 rc = 0,wc = 0,ec = 0,tc = 0;   //请求数统计
        for(size_t i = 0;i < poll_.Size();){
            if(poll_[i].Invalid()){
                poll_.RemoveIndex(i);
                continue;
            }
            const int fd = poll_[i].FD();
            if(poll_[i].Error()){
                errFdVec_.push_back(fd);
                poll_.RemoveIndex(i);
            }else{
                int flags = poll_[i].Flags();
                int revents = 0;
                if(poll_[i].Readable()){
                    DEBUG("poll_["<<i<<"].fd="<<fd<<" can read");
                    flags &= ~POLLRDNORM;
                    revents |= __FdEvent::EVENT_READ;
                    ++rc;
                }
                if(poll_[i].Writable()){
                    DEBUG("poll_["<<i<<"].fd="<<fd<<" can write");
                    flags &= ~POLLWRNORM;
                    revents |= __FdEvent::EVENT_WRITE;
                    ++wc;
                }
                if(revents){
                    const int j = fd % EVENT_QUE_SZ_;
                    DEBUG("push poll_["<<i<<"].fd="<<fd<<", revents="<<revents
                        <<" into eventFdQue_["<<j<<"]");
                    eventList_[j].push_back(__FdEvent(fd,revents));
                    if(flags){
                        DEBUG("modify fd="<<fd<<", events="<<flags);
                        poll_.ModifyIndex(i++,flags,curtime);
                    }else{
                        DEBUG("remove fd="<<fd<<" from poll because events=0");
                        poll_.RemoveIndex(i);
                    }
                }else if(poll_[i].IsExpired(curtime)){
                    expFdVec_.push_back(__FdTime(fd,poll_[i].ExpireTime()));
                    poll_.RemoveIndex(i);
                }else
                    ++i;
            }
        }
        ec += handleErrorFd();
        tc += handleExpiredFd();
        ec += flushEventList();
        if(stats_)
            stats_->Put(rc,wc,ec,tc);
        removeClosedFd();
        addClient(curtime);
    }
    FATAL_COUT("thread quit");
    return 0;
}

void CPollServer::prepare()
{
    LOCAL_LOGGER(logger,"CPollServer::prepare");
    U32 i = Tools::GetMaxFileDescriptor();
    if(i < maxFdSz_){
        Tools::SetMaxFileDescriptor(maxFdSz_);
        maxFdSz_ = Tools::GetMaxFileDescriptor();
    }else
        maxFdSz_ = i;
    INFO("MAX fd size="<<maxFdSz_);
    poll_.Reserve(maxFdSz_);
    eventList_.resize(EVENT_QUE_SZ_);
}

U32 CPollServer::handleErrorFd()
{
    U32 ret = 0;
    if(!errFdVec_.empty()){
        SCOPE_LOGGER(logger,"CPollServer::handleErrorFd");
        __SockPtrVec pSockVec(errFdVec_.size());
        fdSockMap_.CloseSock(errFdVec_.begin(),errFdVec_.end(),pSockVec.begin());
        __FdVec::const_iterator i = errFdVec_.begin();
        __SockPtrVec::const_iterator sock_i = pSockVec.begin();
        for(;i != errFdVec_.end();++i,++sock_i){
            const int & fd = *i;
            const __SockPtr & old = *sock_i;
            DEBUG("poll_[fd="<<fd<<"].sock="<<Tools::ToStringPtr(old)
                <<" error, remove it");
        }
        ret = errFdVec_.size();
        errFdVec_.clear();
    }
    return ret;
}

U32 CPollServer::handleExpiredFd()
{
    typedef Tools::CSelect1st<__FdTime> __ExtFd;
    U32 ret = 0;
    if(!expFdVec_.empty()){    //handle expired fd
        SCOPE_LOGGER(logger,"CPollServer::handleExpiredFd");
        __SockPtrVec pSockVec(expFdVec_.size());
        fdSockMap_.CloseSock(const_iter_adapt(expFdVec_.begin(),__ExtFd())
            ,const_iter_adapt(expFdVec_.end(),__ExtFd())
            ,pSockVec.begin());
        __FdTimeVec::const_iterator i = expFdVec_.begin();
        __SockPtrVec::const_iterator sock_i = pSockVec.begin();
        for(;i != expFdVec_.end();++i,++sock_i){
            WARN("timeout for fd="<<i->first<<", socket="<<Tools::ToStringPtr(*sock_i)
                <<", expireTime="<<Tools::TimeString(i->second)<<", remove it");
        }
        ret = expFdVec_.size();
        expFdVec_.clear();
    }
    return ret;
}

U32 CPollServer::flushEventList()
{
    U32 ret = 0;
    for(int i = 0;i < EVENT_QUE_SZ_;++i){
        if(!eventFdQue_[i].PushAll(eventList_[i],500)){
            SCOPE_LOGGER(logger,"CPollServer::flushEventList");
            ERROR("push all to eventFdQue_["<<i<<"] failed");
            __FdEventList & evList = eventList_[i];
            fdSockMap_.CloseSock(const_iter_adapt_fun<int>(evList.begin(),__FdEvent::ExtractFd)
                ,const_iter_adapt_fun<int>(evList.end(),__FdEvent::ExtractFd));
            for(__FdEventList::const_iterator it = evList.begin();it != evList.end();++it){
                const int & fd = it->fd_;
                ERROR("remove eventList_["<<i<<"].fd="<<fd<<" from poll");
                poll_.RemoveFd(fd);
            }
            ret += evList.size();
            evList.clear();
        }
    }
    return ret;
}

void CPollServer::removeClosedFd()
{
    typedef __FdQue::container_type __Cont;
    __Cont tmp;
    removeFdQue_.PopAll(tmp,0);
    if(!tmp.empty()){
        fdSockMap_.CloseSock(tmp.begin(),tmp.end());
        poll_.RemoveFd(tmp.begin(),tmp.end());
    }
}

void CPollServer::addClient(U32 curtime)
{
    __FdEventList tmp;
    addingFdQue_.PopAll(tmp,0);
    if(!tmp.empty()){
        SCOPE_LOGGER(logger,"CPollServer::addClient");
        __SockPtrVec pSockVec(tmp.size());
        fdSockMap_.GetSock(const_iter_adapt_fun<int>(tmp.begin(),__FdEvent::ExtractFd)
            ,const_iter_adapt_fun<int>(tmp.end(),__FdEvent::ExtractFd)
            ,pSockVec.begin());
        __FdEventList::const_iterator i = tmp.begin();
        __SockPtrVec::const_iterator sock_i = pSockVec.begin();
        for(;i != tmp.end();++i,++sock_i){
            const int & fd = i->fd_;
            const __SockPtr & pSock = *sock_i;
            if(!pSock || pSock->FD() != fd){
                ERROR("fd="<<fd<<" is not pSock="<<Tools::ToStringPtr(pSock)
                    <<" before poll_.AddFd, ignore it");
            }else{
                int ev = 0;
                if(i->Readable())
                    ev |= POLLRDNORM;
                if(i->Writable())
                    ev |= POLLWRNORM;
                poll_.AddFd(fd,ev,curtime);
                DEBUG("poll_.AddFd(client="<<Tools::ToStringPtr(pSock)
                    <<",events="<<i->event_<<", curtime="
                    <<Tools::TimeString(curtime)<<")");
            }
        }
    }
}

NS_SERVER_END

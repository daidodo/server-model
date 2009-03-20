#include <iostream>
#include <common/Logger.h>
#include <common/IterAdapter.h>
#include "EpollServer.h"

NS_SERVER_BEGIN

CEpollServer::CEpollServer(CMainServer & mainServer)
    : CThreadPool(1,mainServer.epollServerStackSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , removeFdQue_(mainServer.removeFdQue_)
    , eventFdQue_(mainServer.eventFdQue_)
    , EVENT_QUE_SZ_(mainServer.tcpServerThreadCount_)
    , stats_(mainServer.serverStatsOn_ ? new __Stats : 0)
{}

CEpollServer::~CEpollServer()
{
    if(stats_)
        delete stats_;
}

#if __USE_EPOLL || defined(__DISPLAY_CODE)

void CEpollServer::Init(const __Config & config)
{
    epoll_.SetFdTimeout(config.GetInt("server.tcp.fd.timeout",0));     //seconds
    epoll_.SetEpollTimeout(config.GetInt("server.epoll.timeout",500)); //ms
    maxFdSz_ = config.GetInt("max.fd.count",1024,64);
}

void CEpollServer::Reconfig(const __Config & config)
{
    epoll_.SetFdTimeout(config.GetInt("server.tcp.fd.timeout",0));     //seconds
    epoll_.SetEpollTimeout(config.GetInt("server.epoll.timeout",500)); //ms
}

void CEpollServer::ShowConfig(std::ofstream & file) const
{
    using namespace std;
    if(!file.is_open())
        return;
    file<<"\nEpollServer :\n"
        <<"  fdTimeout_ = "<<epoll_.GetFdTimeout()<<endl
        <<"  epollTimeout_ = "<<epoll_.GetEpollTimeout()<<endl
        <<"  maxFdSz_ = "<<maxFdSz_<<endl
        ;
}

int CEpollServer::doIt()
{
    LOCAL_LOGGER(logger,"CEpollServer::doIt");
    ASSERT(eventFdQue_ && EVENT_QUE_SZ_ > 0,"eventFdQue_="<<eventFdQue_
        <<", EVENT_QUE_SZ_="<<EVENT_QUE_SZ_<<" invalid");
    prepare();
    for(U32 curtime = 0,check_timeout = time(0);;){
        if(!epoll_.Wait()){
            WARN("epoll error"<<CSocket::ErrMsg());
            continue;
        }
        if(epoll_.CanTimeout())
            curtime = time(0);
        U32 rc = 0,wc = 0,ec = 0;   //请求数统计
        for(size_t i = 0,sz = epoll_.Size();i < sz;++i){
            const int fd = epoll_[i].FD();
            if(epoll_[i].Error()){
                errFdVec_.push_back(fd);
            }else{
                int revents = 0;
                if(epoll_[i].Readable()){
                    DEBUG("epoll_["<<i<<"].fd="<<fd<<" can read");
                    if(epoll_[i].CanExpire()){
                        DEBUG("update epoll_["<<i<<"].fdTime="
                            <<Tools::TimeString(curtime));
                        epoll_[i].SetTime(curtime);
                    }
                    revents |= __FdEvent::EVENT_READ;
                    ++rc;
                }
                if(epoll_[i].Writable()){
                    DEBUG("epoll_["<<i<<"].fd="<<fd<<" can write");
                    revents |= __FdEvent::EVENT_WRITE;
                    ++wc;
                }
                const int j = fd % EVENT_QUE_SZ_;
                DEBUG("push epoll_["<<i<<"].fd="<<fd<<", revents="<<revents
                    <<" into eventFdQue_["<<j<<"]");
                eventList_[j].push_back(__FdEvent(fd,revents));
            }
        }
        ec += handleErrorFd();
        ec += flushEventList();
        removeClosedFd();
        U32 tc = handleExpiredFd(check_timeout,curtime);
        if(stats_)
            stats_->Put(rc,wc,ec,tc);
        addClient(curtime);
    }
    FATAL_COUT("thread quit");
    return 0;
}

void CEpollServer::prepare()
{
    LOCAL_LOGGER(logger,"CEpollServer::prepare");
    U32 i = Tools::GetMaxFileDescriptor();
    if(i < maxFdSz_){
        Tools::SetMaxFileDescriptor(maxFdSz_);
        maxFdSz_ = Tools::GetMaxFileDescriptor();
    }else
        maxFdSz_ = i;
    INFO("MAX fd size="<<maxFdSz_);
    if(!epoll_.Create(maxFdSz_)){
        FATAL_COUT("epoll_.Create(maxFdSz_="<<maxFdSz_<<") error"
            <<CSocket::ErrMsg());
    }
    eventList_.resize(EVENT_QUE_SZ_);
}

U32 CEpollServer::handleErrorFd()
{
    U32 ret = 0;
    if(!errFdVec_.empty()){
        SCOPE_LOGGER(logger,"CEpollServer::handleErrorFd");
        __SockPtrVec pSockVec(errFdVec_.size());
        fdSockMap_.CloseSock(errFdVec_.begin(),errFdVec_.end(),pSockVec.begin());
        __FdVec::const_iterator i = errFdVec_.begin();
        __SockPtrVec::const_iterator sock_i = pSockVec.begin();
        for(;i != errFdVec_.end();++i,++sock_i){
            const int & fd = *i;
            const __SockPtr & old = *sock_i;
            epoll_.RemoveFd(fd);
            WARN("epoll_[fd="<<fd<<"].sock="<<Tools::ToStringPtr(old)
                <<" error, remove it");
        }
        ret = errFdVec_.size();
        errFdVec_.clear();
    }
    return ret;
}

U32 CEpollServer::flushEventList()
{
    U32 ret = 0;
    for(int i = 0;i < EVENT_QUE_SZ_;++i){
        if(!eventFdQue_[i].PushAll(eventList_[i],500)){
            SCOPE_LOGGER(logger,"CEpollServer::flushEventList");
            ERROR("push all to eventFdQue_["<<i<<"] failed, close all sockets");
            __FdEventList & evList = eventList_[i];
            fdSockMap_.CloseSock(const_iter_adapt_fun<int>(evList.begin(),__FdEvent::ExtractFd)
                ,const_iter_adapt_fun<int>(evList.end(),__FdEvent::ExtractFd));
            for(__FdEventList::const_iterator it = evList.begin();it != evList.end();++it){
                const int & fd = it->fd_;
                ERROR("remove eventList_["<<i<<"].fd="<<fd<<" from epoll");
                epoll_.RemoveFd(fd);
            }
            ret += evList.size();
            evList.clear();
        }
    }
    return ret;
}

void CEpollServer::removeClosedFd()
{
    typedef __FdQue::container_type __Cont;
    __Cont tmp;
    removeFdQue_.PopAll(tmp,0);
    if(!tmp.empty()){
        fdSockMap_.CloseSock(tmp.begin(),tmp.end());
        epoll_.RemoveFd(tmp.begin(),tmp.end());
    }
}

U32 CEpollServer::handleExpiredFd(U32 & last_check_time,U32 curtime)
{
    typedef std::pair<int,U32>          __FdTime;
    typedef __DZ_VECTOR(__FdTime)       __FdVec;
    typedef Tools::CSelect1st<__FdTime> __ExtFd;
    U32 ret = 0;
    if(epoll_.CanTimeout() && curtime > last_check_time + epoll_.GetFdTimeout()){
        SCOPE_LOGGER(logger,"CTcpEpollServer::handleExpiredFd");
        __FdVec fdVec;
        for(CEpoll::const_iterator it = epoll_.begin();it != epoll_.end();){
            const int fd = *it++;
            const U32 exptime = epoll_.ExpireTime(fd);
            DEBUG("check fd="<<fd<<", exptime="<<exptime<<"("
                <<Tools::TimeString(exptime)<<"), curtime="<<curtime<<"("
                <<Tools::TimeString(curtime)<<")");
            if(epoll_.IsFdTimeout(fd,curtime))
                fdVec.push_back(__FdTime(fd,exptime));
        }
        if(!fdVec.empty()){
            __SockPtrVec pSockVec(fdVec.size());
            fdSockMap_.CloseSock(const_iter_adapt(fdVec.begin(),__ExtFd())
                ,const_iter_adapt(fdVec.end(),__ExtFd())
                ,pSockVec.begin());
            __FdVec::const_iterator i = fdVec.begin();
            __SockPtrVec::const_iterator sock_i = pSockVec.begin();
            for(;i != fdVec.end();++i,++sock_i){
                WARN("timeout for fd="<<i->first<<", socket="
                    <<Tools::ToStringPtr(*sock_i)<<", expireTime="
                    <<Tools::TimeString(i->second)<<", remove it");
                epoll_.RemoveFd(i->first);
            }
            ret = fdVec.size();
        }
        last_check_time = curtime;
    }
    return ret;
}

void CEpollServer::addClient(U32 curtime)
{
    typedef __FdEventList::const_iterator __Iter;
    __FdEventList tmp;
    addingFdQue_.PopAll(tmp,0);
    if(!tmp.empty()){
        SCOPE_LOGGER(logger,"CEpollServer::addClient");
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
                    <<" before epoll_.AddFd, ignore it");
            }else if(i->Writable()){
                if(!epoll_.ModifyFd(fd,EPOLLIN | EPOLLOUT)){
                    ERROR("epoll_.ModifyFd(client="<<Tools::ToStringPtr(pSock)<<") failed,"
                        <<CSocket::ErrMsg()<<", close it");
                    fdSockMap_.SetSock(fd,0);
                }else{
                    DEBUG("epoll_.ModifyFd(client="<<Tools::ToStringPtr(pSock)
                        <<") for read and write");
                }
            }else if(!epoll_.AddFd(fd,EPOLLIN,curtime)){
                ERROR("epoll_.AddFd(client="<<Tools::ToStringPtr(pSock)<<") failed,"
                    <<CSocket::ErrMsg()<<", close it");
                fdSockMap_.SetSock(fd,0);
            }else{
                DEBUG("epoll_.AddFd(client="<<Tools::ToStringPtr(pSock)
                    <<", curtime="<<Tools::TimeString(curtime)<<") for read");
            }
        }
    }
}

#else

void CEpollServer::Init(const __Config & config){}

void CEpollServer::Reconfig(const __Config & config){}

void CEpollServer::ShowConfig(std::ofstream & file) const{}

int CEpollServer::doIt()
{
    std::cerr<<"epoll not supported\n";
    abort();
    return 0;
}

#endif

NS_SERVER_END

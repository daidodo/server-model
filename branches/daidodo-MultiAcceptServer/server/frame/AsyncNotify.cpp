#include <Logger.h>
#include <Tools.h>
#include <IterAdapter.h>

#include "AsyncNotify.h"

NS_SERVER_BEGIN

CAsyncNotify::CAsyncNotify(const CNotifyCtorParams & params)
    : CThreadPool(1, params.stackSize_)
    , addingQue_(params.addingQue_)
    , eventQue_(params.eventQue_)
    , fdSockMap_(params.fdSockMap_)
{}

CAsyncNotify::~CAsyncNotify()
{
}

bool CAsyncNotify::Init(const CNotifyInitParams & params)
{
    if(!initEpoll(params.maxFdNum_))
        return false;
    epoll_.SetFdTimeout(params.fdTimeoutS_);     //seconds
    epoll_.SetEpollTimeout(params.epollTimeoutMs_);     //milliseconds
    return true;
}

int CAsyncNotify::doIt()
{
    LOCAL_LOGGER(logger, "CAsyncNotify::doIt");
    __FdEventList fdEventList;
    __FdList errFdList;
    U32 curtime;
    for(;;){
        if(!epoll_.Wait()){
            ERROR("epoll wait error"<<CSocket::ErrMsg());
            continue;
        }
        curtime = time(0);
        for(size_t i = 0, sz = epoll_.Size();i < sz;++i){
            CEpollEvent event = epoll_[i];
            const int fd = event.Fd();
            if(!event.IsValid()){
                WARN("epoll_["<<i<<"]="<<event.ToString()<<" is invalid");
                continue;
            }else if(event.Error()){
                WARN("epoll_["<<i<<"]="<<event.ToString()<<" is error");
                errFdList.push_back(fd);
            }else{
                int revents = 0;
                if(epoll_[i].Readable()){
                    DEBUG("epoll_["<<i<<"]="<<event.ToString()<<" can read");
                    revents |= __FdEvent::EVENT_READ;
                }
                if(epoll_[i].Writable()){
                    DEBUG("epoll_["<<i<<"]="<<event.ToString()<<" can write");
                    revents |= __FdEvent::EVENT_WRITE;
                }
                DEBUG("push epoll_["<<i<<"]="<<event.ToString()<<", revents="<<revents<<" into fdEventList");
                fdEventList.push_back(__FdEvent(fd, revents));
            }
        }
        addFdEvent(curtime, errFdList);
        handleExpiredFd();
        handleErrorFd();
    }
    return 0;
}

bool CAsyncNotify::initEpoll(U32 maxFdNum)
{
    LOCAL_LOGGER(logger,"CAsyncNotify::initEpoll");
    U32 i = Tools::GetMaxFileDescriptor();
    if(i < maxFdNum){
        Tools::SetMaxFileDescriptor(maxFdNum);
        i = Tools::GetMaxFileDescriptor();
    }
    if(i != maxFdNum){
        WARN("real MAX fd size="<<i<<" is not maxFdNum="<<maxFdNum);
        maxFdNum = i;
    }else{
        INFO("MAX fd size="<<maxFdNum);
    }
    if(!epoll_.Create(maxFdNum)){
        FATAL_COUT("epoll_.Create(maxFdNum="<<maxFdNum<<") error"<<CSocket::ErrMsg());
        return false;
    }
    return true;
}

void CAsyncNotify::addFdEvent(U32 curtime, __FdList & errFdList)
{
    SCOPE_LOGGER(logger, "CAsyncNotify::addFdEvent");
    __FdEventList tmp;
    if(!addingFdQue_.PopAll(tmp,0)){
        WARN("addingFdQue_.PopAll() failed");
        return;
    }
    if(tmp.empty())
        return;
    __SockPtrList sockList(tmp.size());
    fdSockMap_.GetSock(const_iter_adapt_fun<int>(tmp.begin(), __FdEvent::ExtractFd), const_iter_adapt_fun<int>(tmp.end(), __FdEvent::ExtractFd), sockList.begin());
    __FdEventList::const_iterator i = tmp.begin();
    __SockPtrList::const_iterator sock_i = sockList.begin();
    for(;i != tmp.end();++i, ++sock_i){
        const int fd = i->Fd();
        const __SockPtr & sock = *sock_i;
        if(!sock || sock->Fd() != fd){
            ERROR("fd="<<fd<<" is not sock="<<Tools::ToStringPtr(sock)<<" before add to epoll, ignore it");
        }else if(i->Closable()){
            errFdList.push_back(fd);
        }else{
            U32 ev = 0;
            if(i->Writable())
                ev |= EPOLLOUT;
            if(i->Readable())
                ev |= EPOLLIN;
            if(epoll_.AddOrModifyFd(fd, ev, curtime)){
                DEBUG("epoll_.AddOrModifyFd(fd="<<fd<<", ev="<<ev<<", client="<<Tools::ToStringPtr(pSock)<<") succ");
            }else{
                ERROR("epoll_.AddOrModifyFd(fd="<<fd<<", ev="<<ev<<", client="<<Tools::ToStringPtr(pSock)<<") failed,"<<CSocket::ErrMsg()<<", close it");
                errFdList.push_back(fd);
            }
        }
    }
}

void CAsyncNotify::handleExpiredFd()
{
}

void CAsyncNotify::handleErrorFd()
{
}


NS_SERVER_END

#include <Logger.h>
#include <Tools.h>
#include <Sockets.h>
#include <IterAdapter.h>

#include "HahsEngine.h"
#include "AsyncNotify.h"

NS_SERVER_BEGIN

CAsyncNotify::CAsyncNotify(size_t stackSz, CHahsEngine & engine)
    : CThreadPool(1, stackSz)
    , addingQue_(engine.addingQue_)
    , eventQue_(engine.eventQue_)
    , fdSockMap_(engine.fdSockMap_)
{}

bool CAsyncNotify::Init(U32 maxFdNum, int epollTimeoutMs)
{
    if(!initEpoll(maxFdNum))
        return false;
    epoll_.EpollTimeout(epollTimeoutMs);//milliseconds
    return true;
}

int CAsyncNotify::doIt()
{
    LOCAL_LOGGER(logger, "CAsyncNotify::doIt");
    __FdEventList fdEventList;
    __FdArray errFdList;
    for(;;){
        if(!epoll_.Wait()){
            ERROR("epoll wait error"<<CSocket::ErrMsg());
            continue;
        }
        //handle events
        for(size_t i = 0, sz = epoll_.Size();i < sz;++i){
            CEpollEvent event = epoll_[i];
            const int fd = event.Fd();
            if(event.Invalid()){
                WARN("epoll_["<<i<<"]="<<event.ToString()<<" is invalid");
                continue;
            }else if(event.Error()){
                WARN("epoll_["<<i<<"]="<<event.ToString()<<" is error");
                errFdList.push_back(fd);
            }else{
                __Events revents = 0;
                if(epoll_[i].CanInput()){
                    DEBUG("epoll_["<<i<<"]="<<event.ToString()<<" can input");
                    revents |= EVENT_IN;
                }
                if(epoll_[i].CanOutput()){
                    DEBUG("epoll_["<<i<<"]="<<event.ToString()<<" can output");
                    revents |= EVENT_OUT;
                }
                DEBUG("push epoll_["<<i<<"]="<<event.ToString()<<", revents="<<revents<<" into fdEventList");
                fdEventList.push_back(__FdEvent(fd, revents));
            }
        }
        //add events
        if(!eventQue_.PushAll(fdEventList, 500)){
            errFdList.insert(errFdList.end()
                    , const_iter_adapt_fun<int>(fdEventList.begin(), __FdEvent::ExtractFd)
                    , const_iter_adapt_fun<int>(fdEventList.end(), __FdEvent::ExtractFd));
        }
        fdEventList.clear();
        //add sockets
        addFdEvent(errFdList);
        //close sockets
        fdSockMap_.CloseSock(errFdList.begin(), errFdList.end());
        epoll_.RemoveFd(errFdList.begin(), errFdList.end());
        errFdList.clear();
    }
    return 0;
}

bool CAsyncNotify::initEpoll(U32 maxFdNum)
{
    LOCAL_LOGGER(logger,"CAsyncNotify::initEpoll");
    //adjust max file num
    U32 i = Tools::GetMaxFileDescriptor();
    if(i < maxFdNum){
        Tools::SetMaxFileDescriptor(maxFdNum);
        i = Tools::GetMaxFileDescriptor();
    }
    if(i != maxFdNum){
        WARN("real MAX fd num="<<i<<" is not maxFdNum="<<maxFdNum);
        maxFdNum = i;
    }else{
        INFO("MAX fd num="<<maxFdNum);
    }
    //create epoll
    if(!epoll_.Create(maxFdNum)){
        FATAL_COUT("epoll_.Create(maxFdNum="<<maxFdNum<<") error"<<CSocket::ErrMsg());
        return false;
    }
    return true;
}

void CAsyncNotify::addFdEvent(__FdArray & errFdList)
{
    LOCAL_LOGGER(logger, "CAsyncNotify::addFdEvent");
    //pop all
    __FdList tmp;
    if(!addingQue_.PopAll(tmp, 0))
        return;
    if(tmp.empty())
        return;
    //get sock ptr
    __SockPtrList sockList(tmp.size());
    fdSockMap_.GetSock(tmp.begin(), tmp.end(), sockList.begin());
    __FdList::const_iterator i = tmp.begin();
    __SockPtrList::const_iterator sock_i = sockList.begin();
    for(;i != tmp.end();++i, ++sock_i){
        const int fd = *i;
        const __SockPtr & sock = *sock_i;
        //validate fd and sock ptr
        if(!sock || !sock->IsValid()){
            ERROR("fd="<<fd<<" is not sock="<<Tools::ToStringPtr(sock)<<" before add to epoll, ignore it");
            continue;
        }else if(Events::NeedClose(sock->Events())){
            errFdList.push_back(fd);
            continue;
        }
        //add fd and events to epoll
        U32 ev = 0;     //epoll flags
        if(Events::NeedInput(sock->Events()))
            ev |= EPOLLIN;
        if(Events::NeedOutput(sock->Events()))
            ev |= EPOLLOUT;
        if(!epoll_.ModifyFlags(fd, ev)){
            WARN("epoll_.ModFlags(fd="<<fd<<", ev="<<ev<<") failed for client="<<Tools::ToStringPtr(sock)<<", close it");
            errFdList.push_back(fd);
        }
    }
}

NS_SERVER_END

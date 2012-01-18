#include <Logger.h>
#include <Tools.h>
#include <Sockets.h>
#include <IterAdapter.h>

#include "AsyncNotify.h"

NS_SERVER_BEGIN

CAsyncNotify::CAsyncNotify(const CNotifyCtorParams & params)
    : CThreadPool(1, params.stackSize_)
    , addingQue_(params.addingQue_)
    , eventQue_(params.eventQue_)
    , fdSockMap_(params.fdSockMap_)
{}

bool CAsyncNotify::Init(const CNotifyInitParams & params)
{
    if(!initEpoll(params.maxFdNum_))
        return false;
    epoll_.EpollTimeout(params.epollTimeoutMs_);//milliseconds
    return true;
}

int CAsyncNotify::doIt()
{
    LOCAL_LOGGER(logger, "CAsyncNotify::doIt");
    __FdEventList fdEventList;
    __FdList errFdList;
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
        //add events
        if(!eventQue_.PushAll(fdEventList, 500)){
            errFdList.insert(errFdList.end()
                    , const_iter_adapt_fun<int>(fdEventList.begin(), __FdEvent::ExtractFd)
                    , const_iter_adapt_fun<int>(fdEventList.end(), __FdEvent::ExtractFd));
            fdEventList.clear();
        }
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

void CAsyncNotify::addFdEvent(__FdList & errFdList)
{
    LOCAL_LOGGER(logger, "CAsyncNotify::addFdEvent");
    //pop all
    __FdEventList tmp;
    if(!addingQue_.PopAll(tmp, 0)){
        WARN("addingQue_.PopAll() failed");
        return;
    }
    if(tmp.empty())
        return;
    //get sock ptr
    __SockPtrList sockList(tmp.size());
    fdSockMap_.GetSock(const_iter_adapt_fun<int>(tmp.begin(), __FdEvent::ExtractFd), const_iter_adapt_fun<int>(tmp.end(), __FdEvent::ExtractFd), sockList.begin());
    __FdEventList::const_iterator i = tmp.begin();
    __SockPtrList::const_iterator sock_i = sockList.begin();
    for(;i != tmp.end();++i, ++sock_i){
        const int fd = i->Fd();
        const __SockPtr & sock = *sock_i;
        //validate fd and sock ptr
        if(!sock || sock->Fd() != fd){
            ERROR("fd="<<fd<<" is not sock="<<Tools::ToStringPtr(sock)<<" before add to epoll, ignore it");
            continue;
        }else if(i->Closable()){
            errFdList.push_back(fd);
            continue;
        }
        //add fd and event to epoll
        U32 ev = 0;
        if(i->Readable())
            ev |= EPOLLIN;
        if(i->Writable())
            ev |= EPOLLOUT;
        if(!epoll_.ModifyFlags(fd, ev, i->AddFlags())){
            WARN("epoll_.ModFlags(fd="<<fd<<", ev="<<ev<<", add="<<i->AddFlags()<<") failed for client="<<Tools::ToStringPtr(sock)<<", close it");
            errFdList.push_back(fd);
        }
        //update sock event flags
        int rev = 0;
        ev = epoll_.GetFlags(fd);
        if(0 != (ev & EPOLLIN))
            rev |= __FdEvent::EVENT_READ;
        if(0 != (ev & EPOLLOUT))
            rev |= __FdEvent::EVENT_WRITE;
        sock->EventFlags(rev);
    }
}

NS_SERVER_END

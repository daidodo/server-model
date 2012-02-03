#include <sstream>

#include "Epoll.h"

NS_SERVER_BEGIN

//struct CEpollEvent

std::string CEpollEvent::ToString() const
{
    std::ostringstream oss;
    oss<<"{fd="<<ev_.data.fd
        <<", events="<<CEpoll::EventsName(ev_.events)
        <<"}";
    return oss.str();
}

//class CEpoll

CEpoll::CEpoll()
    : fdSize_(0)
    , epollFd_(-1)
    , maxSz_(0)
    , epollTimeoutMs_(400)
{}

bool CEpoll::addFdFlags(int fd, U32 flags)
{
    __Event ev;
    memset(&ev, 0, sizeof ev);
    ev.events = flags | EPOLLET;
    ev.data.fd = fd;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0)
        return false;
    assert(0 == fdInfo_[fd]);
    fdInfo_[fd] = flags;
    ++fdSize_;
    return true;
}

bool CEpoll::modifyFdFlags(int fd, U32 flags)
{
    U32 & oldFlags = fdInfo_[fd];
    assert(0 != oldFlags);
    if(flags != oldFlags){
        __Event ev;
        memset(&ev, 0, sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd = fd;
        if(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) != 0)
            return false;
        oldFlags = flags;
    }
    return true;
}

NS_SERVER_END

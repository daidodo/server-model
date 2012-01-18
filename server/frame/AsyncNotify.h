#ifndef DOZERG_ASYNC_NOTIFY_H_20120110
#define DOZERG_ASYNC_NOTIFY_H_20120110

#include <Threads.h>
#include <Epoll.h>

#include "Structs.h"

NS_SERVER_BEGIN

struct CNotifyCtorParams
{
    size_t stackSize_;
    __FdEventQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
};

struct CNotifyInitParams
{
    U32 maxFdNum_;
    int epollTimeoutMs_; //milliseconds
};

struct CAsyncNotify : public CThreadPool
{
    //functions
    explicit CAsyncNotify(const CNotifyCtorParams & params);
    bool Init(const CNotifyInitParams & params);
protected:
    virtual int doIt();
private:
    bool initEpoll(U32 maxFdNum);
    void addFdEvent(__FdList & errFdList);
    //members
    __FdEventQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    CEpoll epoll_;
};

NS_SERVER_END

#endif


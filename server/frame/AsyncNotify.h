#ifndef DOZERG_ASYNC_NOTIFY_H_20120110
#define DOZERG_ASYNC_NOTIFY_H_20120110

#include <Threads.h>
#include <Epoll.h>

#include "Structs.h"

NS_SERVER_BEGIN

class CHahsEngine;

struct CAsyncNotify : public CThreadPool
{
    //functions
    CAsyncNotify(size_t stackSz, CHahsEngine & engine);
    bool Init(U32 maxFdNum, int epollTimeoutMs);
protected:
    virtual int doIt();
private:
    bool initEpoll(U32 maxFdNum);
    void addFdEvent(__FdArray & errFdList);
    //members
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    CEpoll epoll_;
};

NS_SERVER_END

#endif


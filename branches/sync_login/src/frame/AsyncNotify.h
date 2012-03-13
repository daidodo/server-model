#ifndef DOZERG_ASYNC_NOTIFY_H_20120110
#define DOZERG_ASYNC_NOTIFY_H_20120110

#include <vector>

#include <Threads.h>
#include <Epoll.h>

#include "HahsEngine.h"

NS_SERVER_BEGIN

struct CAsyncNotify : public CThreadPool
{
    typedef std::vector<int> __FdArray;
    typedef CHahsEngine::__FdSockMap __FdSockMap;
    typedef CHahsEngine::__FdQue __FdQue;
    typedef CHahsEngine::__FdEventQue __FdEventQue;
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


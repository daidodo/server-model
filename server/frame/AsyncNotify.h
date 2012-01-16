#ifndef DOZERG_ASYNC_NOTIFY_H_20120110
#define DOZERG_ASYNC_NOTIFY_H_20120110

#include <Threads.h>
#include <LockQueue.h>
#include <SharedPtr.h>
#include <FdMap.h>
#include <Epoll.h>

#include "Structs.h"

NS_SERVER_BEGIN

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef CSharedPtr<CCmdSock> __SockPtr;
typedef CFdSockMap<CCmdSock, __SockPtr> __FdSockMap;
typedef __FdEventQue::container_type __FdEventList;
typedef std::vector<int> __FdList;
typedef std::vector<__SockPtr> __SockPtrList;

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
    U32 fdTimeoutS_;     //seconds
    int epollTimeoutMs_; //milliseconds
};

class CAsyncNotify : public CThreadPool
{
public:
    //functions
    explicit CAsyncNotify(const CNotifyCtorParams & params);
    ~CAsyncNotify();
    bool Init(const CNotifyInitParams & params);
protected:
    virtual int doIt();
private:
    bool initEpoll(int maxFdNum);
    void addFdEvent(U32 curtime, __FdList & errFdList);
    void handleExpiredFd();
    void handleErrorFd();
    //members
    __FdEventQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    CEpoll epoll_;


};

NS_SERVER_END

#endif


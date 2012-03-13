#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)架构的服务器引擎
        CHahsEngine
//*/

#include <SharedPtr.h>
#include <FdMap.h>
#include <LockQueue.h>

#include "SockSession.h"

NS_SERVER_BEGIN

class CAsyncNotify;
class CAsyncIO;
class CCmdHandler;

struct CHahsEnginParams
{
    U32 maxFdNum_;
    int epollTimeoutMs_;
    size_t notifyStackSz_;
    size_t ioStackSz_;
    bool needHandler_;
    size_t handlerStackSz_;
    int handlerThreadMax_;
};

class CCmdSession;

class CHahsEngine
{
    friend class CAsyncNotify;
    friend class CAsyncIO;
    friend class CCmdHandler;
    typedef CSockSession __SockSession;
    typedef CSharedPtr<__SockSession> __SockPtr;
    typedef CCmdSession __CmdSession;
    typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;
    typedef CLockQueue<int> __FdQue;
    typedef CLockQueue<CFdEvent> __FdEventQue;
public:
    typedef CLockQueue<__CmdSession *> __QueryCmdQue;
    //functions
    CHahsEngine();
    ~CHahsEngine(){uninit();}
    //增加tcp监听socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper);
    //增加tcp主动连接socket
    bool AddTcpConn(const CSockAddr & connectAddr, const CRecvHelper & recvHelper);
    //增加udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr, const CRecvHelper & recvHelper);
    //增加文件，flags指定读或者写
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
    //创建线程，开始运行
    bool Run(const CHahsEnginParams & params);
    //等待所有线程退出
    void WaitAll();
private:
    void uninit();
    bool addSock(__SockPtr & sock, __Events ev);
    //members
    __FdQue addingQue_;
    __FdEventQue eventQue_;
    __FdSockMap fdSockMap_;
    __QueryCmdQue queryCmdQue_;
    CAsyncNotify * notify_;
    CAsyncIO * io_;
    CCmdHandler * handler_;
};

NS_SERVER_END

#endif


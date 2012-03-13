#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)�ܹ��ķ���������
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
    //����tcp����socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper);
    //����tcp��������socket
    bool AddTcpConn(const CSockAddr & connectAddr, const CRecvHelper & recvHelper);
    //����udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr, const CRecvHelper & recvHelper);
    //�����ļ���flagsָ��������д
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
    //�����̣߳���ʼ����
    bool Run(const CHahsEnginParams & params);
    //�ȴ������߳��˳�
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


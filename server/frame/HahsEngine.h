#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)�ܹ��ķ���������
        CHahsEngine
//*/

#include <Sockets.h>
#include "Structs.h"

NS_SERVER_BEGIN

class CAsyncNotify;
class CAsyncIO;
class CCmdHandler;

struct CHahsEnginParams
{
    size_t notifyStackSz_;
    size_t ioStackSz_;
    size_t handlerStackSz_;
    U32 maxFdNum_;
    int epollTimeoutMs_;
    int handlerThreadMax_;
};

class CHahsEngine
{
    friend class CAsyncNotify;
    friend class CAsyncIO;
    friend class CCmdHandler;
public:
    CHahsEngine();
    ~CHahsEngine();
    //����tcp����socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper);
    //����tcp��������socket
    bool AddTcpConn(const CSockAddr & connectAddr);
    //����udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr);
    //�����ļ���flagsָ��������д
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
    //�����̣߳���ʼ����
    bool Run(const CHahsEnginParams & params);
    //�ȴ������߳��˳�
    void WaitAll();
private:
    void uninit();
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


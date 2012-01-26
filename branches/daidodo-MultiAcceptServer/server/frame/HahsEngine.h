#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)架构的服务器引擎
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
    //增加tcp监听socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper);
    //增加tcp主动连接socket
    bool AddTcpConn(const CSockAddr & connectAddr);
    //增加udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr);
    //增加文件，flags指定读或者写
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
    //创建线程，开始运行
    bool Run(const CHahsEnginParams & params);
    //等待所有线程退出
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


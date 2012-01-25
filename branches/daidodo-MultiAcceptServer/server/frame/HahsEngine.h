#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)架构的服务器引擎
        CHahsEngine
//*/

#include <Sockets.h>
#include "SockSession.h"

NS_SERVER_BEGIN

struct CHahsEngine
{
    //增加tcp监听socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper)
    {
        return true;
    }
    //增加tcp主动连接socket
    bool AddTcpConn(const CSockAddr & connectAddr);
    //增加udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr);
    //增加文件，flags指定读或者写
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
};


NS_SERVER_END

#endif


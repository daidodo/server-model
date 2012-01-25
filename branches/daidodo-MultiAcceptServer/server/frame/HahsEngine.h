#ifndef DOZERG_HAHS_ENGINE_H_20120119
#define DOZERG_HAHS_ENGINE_H_20120119

/*
    HA/HS(half-async/half-sync)�ܹ��ķ���������
        CHahsEngine
//*/

#include <Sockets.h>
#include "SockSession.h"

NS_SERVER_BEGIN

struct CHahsEngine
{
    //����tcp����socket
    bool AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper)
    {
        return true;
    }
    //����tcp��������socket
    bool AddTcpConn(const CSockAddr & connectAddr);
    //����udp socket
    bool AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr);
    //�����ļ���flagsָ��������д
    bool AddFile(const std::string & pathname, int flags, mode_t mode);
};


NS_SERVER_END

#endif


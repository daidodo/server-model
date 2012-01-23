#ifndef DOZERG_CMD_SESSION_H_20120122
#define DOZERG_CMD_SESSION_H_20120122

#include <memory>

#include <LockQueue.h>
#include <Sockets.h>
#include "SockSession.h"

NS_SERVER_BEGIN

struct CCmdSession
{
    typedef std::allocator<CCmdSession> allocator_type;
    static CCmdSession * GetObject(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr)
    {
        CCmdSession * ret = allocator_type().allocate(1);
        return new (ret) CCmdSession(sock, cmd, udpClientAddr);
    }
    CCmdSession(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr)
        : sock_(sock)
        , cmd_(cmd)
    {
        udpClientAddr_.swap(udpClientAddr);
    }
    __SockPtr & SockPtr(){return sock_;}
    __CmdBase * CmdBase(){return cmd_;}
    const CSockAddr & UdpClientAddr() const{return udpClientAddr_;}
    std::string ToString() const{return "";}
private:
    __SockPtr sock_;
    __CmdBase * cmd_;
    CSockAddr udpClientAddr_;
};

typedef CCmdSession __CmdSession;
typedef CLockQueue<__CmdSession *> __QueryCmdQue;

NS_SERVER_END

#endif


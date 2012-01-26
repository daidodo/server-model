#ifndef DOZERG_CMD_SESSION_H_20120122
#define DOZERG_CMD_SESSION_H_20120122

#include <memory>

#include <LockQueue.h>
#include "SockSession.h"

NS_SERVER_BEGIN

class CCmdSession
{
    typedef CSockSession::__RecvHelper __RecvHelper;
public:
    typedef std::allocator<CCmdSession> allocator_type;
    //functions
    static CCmdSession * GetObject(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr);
    CCmdSession(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr);
    ~CCmdSession();
    __SockPtr & SockPtr(){return sock_;}
    __CmdBase * CmdBase(){return cmd_;}
    CSockAddr & UdpClientAddr(){return udpClientAddr_;}
    std::string ToString() const;
private:
    //members
    __SockPtr sock_;
    __CmdBase * cmd_;
    CSockAddr udpClientAddr_;
};

typedef CCmdSession __CmdSession;
typedef CLockQueue<__CmdSession *> __QueryCmdQue;

NS_SERVER_END

#endif


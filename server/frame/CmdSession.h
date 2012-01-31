#ifndef DOZERG_CMD_SESSION_H_20120122
#define DOZERG_CMD_SESSION_H_20120122

#include <memory>

#include "SockSession.h"

NS_SERVER_BEGIN

struct CCmdSession
{
    typedef std::allocator<CCmdSession> allocator_type;
    //functions
    static CCmdSession * GetObject(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr);
    ~CCmdSession();
    const __SockPtr & SockPtr() const{return sock_;}
    __CmdBase * CmdBase() const{return cmd_;}
    CSockAddr & UdpClientAddr(){return udpClientAddr_;}
    std::string ToString() const;
private:
    CCmdSession(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr);
	CCmdSession(const CCmdSession &);
	CCmdSession operator =(const CCmdSession &);
    //members
    const __SockPtr sock_;
    __CmdBase * const cmd_;
    CSockAddr udpClientAddr_;
};

typedef CCmdSession __CmdSession;

NS_SERVER_END

#endif


#include <sstream>

#include <Tools.h>
#include "Command.h"
#include "CmdSession.h"

NS_SERVER_BEGIN

CCmdSession * CCmdSession::GetObject(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr)
{
    CCmdSession * ret = allocator_type().allocate(1);
    return new (ret) CCmdSession(sock, cmd, udpClientAddr);
}

CCmdSession::CCmdSession(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr)
    : sock_(sock)
    , cmd_(cmd)
{
    udpClientAddr_.Swap(udpClientAddr);
}

CCmdSession::~CCmdSession()
{
    if(cmd_){
        assert(sock_);
        sock_->ReleaseCmd(cmd_);
        cmd_ = 0;
    }
    sock_ = 0;
}

std::string CCmdSession::ToString() const
{
    std::ostringstream oss;
    oss<<"{sock_="<<Tools::ToStringPtr(sock_)
        <<", cmd_="<<Tools::ToStringPtr(cmd_)
        <<", udpClientAddr_="<<udpClientAddr_.ToString()
        <<"}";
    return oss.str();
}

NS_SERVER_END

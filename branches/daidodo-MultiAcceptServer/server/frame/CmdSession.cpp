#include <sstream>

#include <Tools.h>
#include "Command.h"
#include "CmdSession.h"

NS_SERVER_BEGIN

CCmdSession * CCmdSession::GetObject(int fd, U32 fingerPrint, __CmdBase * cmd, const CRecvHelper & recvHelper, CSockAddr & udpClientAddr)
{
    CCmdSession * ret = allocator_type().allocate(1);
    return new (ret) CCmdSession(fd, fingerPrint, cmd, recvHelper, udpClientAddr);
}

CCmdSession::CCmdSession(int fd, U32 fingerPrint, __CmdBase * cmd, const CRecvHelper & recvHelper, CSockAddr & udpClientAddr)
    : fd_(fd)
    , finger_(fingerPrint)
    , cmd_(cmd)
    , recvHelper_(recvHelper)
{
    udpClientAddr_.Swap(udpClientAddr);
}

CCmdSession::~CCmdSession()
{
    if(cmd_)
        recvHelper_.ReleaseCmd(cmd_);
}

std::string CCmdSession::ToString() const
{
    std::ostringstream oss;
    oss<<"{fd_="<<fd_
        <<", finger_="<<finger_
        <<", cmd_="<<Tools::ToStringPtr(cmd_)
        <<", udpClientAddr_="<<udpClientAddr_.ToString()
        <<"}";
    return oss.str();
}

NS_SERVER_END

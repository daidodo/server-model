#include <sstream>

#include "SockSession.h"
#include "RecvHelper.h"

NS_SERVER_BEGIN

//class CSockHanle

std::string CSockHanle::ToString() const
{
    std::ostringstream oss;
    oss<<"{sock_="<<sock_.ToString()
        <<", udpClientAddr_="<<udpClientAddr_.ToString()
        <<"}";
    return oss.str();
}

bool CSockHanle::AddOutBuf(__Buffer & buf)
{
    return sock_.AddOutBuf(buf, udpClientAddr_);
}

//class CRecvHelper

void CRecvHelper::ReleaseCmd(const CAnyPtr & cmd) const
{}

__Events CRecvHelper::ProcessCmd(const CAnyPtr & cmd, CSockHanle & handle) const
{
    return 0;
}

std::string CRecvHelper::ToString() const
{
    std::ostringstream oss;
    oss<<"{@"<<this<<"}";
    return oss.str();
}

NS_SERVER_END

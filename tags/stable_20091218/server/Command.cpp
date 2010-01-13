#include <sstream>
#include <common/impl/Alloc.h>
#include <common/Logger.h>
#include <common/Tools.h>
#include <common/EncryptorAes.h>
#include "Command.h"

NS_SERVER_BEGIN

//struct CQueryCmd
__DZ_STRING CQueryCmd::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<"(fileHash_="<<Tools::DumpHex(fileHash_)
        <<",clientHash_="<<Tools::DumpHex(clientHash_)
        <<",peerId_="<<peerId_
        <<")";
    return oss.str();
}

bool CQueryCmd::DecodeParam(CInByteStream & ds)
{
    LOCAL_LOGGER(logger,"CQueryCmd::DecodeParam");
    ds>>fileHash_>>clientHash_>>peerId_;
    DEBUG(ToString());
    return ds;
}

//struct CQueryRespCmd
CQueryRespCmd::CQueryRespCmd(const QCmdBase & head)
    : RCmdBase(CMD_RESP,head)
{}

__DZ_STRING CQueryRespCmd::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<"(result_="<<U32(result_)
        <<",fileHash_="<<Tools::DumpHex(fileHash_)
        <<",clientHash_="<<Tools::DumpHex(clientHash_);
    for(size_t i = 0;i < peerId_.size();++i)
        oss<<",peerId_["<<i<<"]="<<peerId_[i];
    oss<<")";
    return oss.str();
}

void CQueryRespCmd::EncodeParam(COutByteStream & ds) const
{
    LOCAL_LOGGER(logger,"CQueryRespCmd::EncodeParam");
    DEBUG(ToString());
    ds<<result_<<fileHash_<<clientHash_
        <<Manip::array(&peerId_[0],peerId_.size());
}

NS_SERVER_END

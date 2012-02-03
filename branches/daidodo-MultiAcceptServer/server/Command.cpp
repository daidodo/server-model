#include <sstream>

#include <Logger.h>
#include "Command.h"

NS_SERVER_BEGIN

//class CCmdBase

//wLen cStx wCmdId body cEtx
CCmdBase * CCmdBase::DecodeCmd(const char * buf, size_t sz)
{
    LOCAL_LOGGER(logger, "CCmdBase::DecodeCmd");
    assert(buf && sz);
    CInByteStream in(buf, sz);
    //validate
    U16 totalLen = 0;
    U8 stx = 0;
    U16 cmdId = 0;
    if(!(in>>totalLen>>stx>>cmdId)){
        ERROR("decode buf="<<Tools::Dump(buf, sz)<<" error");
        return 0;
    }
    U8 etx = buf[sz - 1];
    if(sz != totalLen
            || STX != stx
            || ETX != etx){
        ERROR("check totalLen="<<totalLen<<", stx="<<(int)stx<<", etx="<<(int)etx<<" failed for buf="<<Tools::Dump(buf, sz));
        return 0;
    }
    //decode
    switch(cmdId){
        case CMD_QUERY:{
            CCmdQuery * q = new CCmdQuery;
            if(!q->Decode(in)){
                ERROR("decode CMD_QUERY failed for buf="<<Tools::Dump(buf, sz));
                return 0;
            }
            return q;}
        default:
            ERROR("cmdId="<<cmdId<<" is invalid in buf="<<Tools::Dump(buf, sz));
    }
    return 0;
}

void CCmdBase::ReleaseCmd(CCmdBase * cmd)
{
    assert(cmd);
    switch(cmd->CmdId()){
        case CMD_QUERY:delete cmd;break;
        default:break;
    }
}

std::string CCmdBase::ToString() const
{
    std::ostringstream oss;
    switch(cmdId_){
        case CMD_QUERY:oss<<"CMD_QUERY";break;
        case CMD_RESP:oss<<"CMD_RESP";break;
        default:oss<<"UNKNOWN["<<cmdId_<<"]";break;
    }
    return oss.str();
}

//struct CCmdRecvHelper

CCmdRecvHelper::__OnDataArriveRet CCmdRecvHelper::OnDataArrive(const char * buf, size_t sz) const
{
    assert(buf);
    if(sz < 5)
        return __OnDataArriveRet(RR_NEED_MORE, 5 - sz);
    U16 totalLen = 0;
    CInByteStream in(buf, sz);
    in>>totalLen;
    if(totalLen > 1024)
        return __OnDataArriveRet(RR_ERROR, 0);
    if(sz >= totalLen)
        return __OnDataArriveRet(RR_COMPLETE, sz - totalLen);
    return __OnDataArriveRet(RR_NEED_MORE, totalLen - sz);
}

bool CCmdRecvHelper::HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const
{
    CCmdBase * base = CCmdBase::DecodeCmd(buf, sz);
    if(base){
        cmd = base;
        return true;
    }
    return false;
}

void CCmdRecvHelper::ReleaseCmd(const CAnyPtr & cmd) const
{
    assert(cmd);
    CCmdBase * base = PtrCast<CCmdBase>(cmd);
    assert(base);
    CCmdBase::ReleaseCmd(base);
}

//struct CCmdQuery

bool CCmdQuery::Decode(CInByteStream & in)
{
    return (in>>ver_>>echo_);
}

std::string CCmdQuery::ToString() const
{
    std::ostringstream oss;
    oss<<"{base="<<CCmdBase::ToString()
        <<", ver_="<<ver_
        <<", echo_="<<Tools::Dump(echo_)
        <<"}";
    return oss.str();
}

//struct CCmdResp

void CCmdResp::Encode(COutByteStream & out) const
{
    size_t from = out.Size();
    out<<U16(0)     //total len
        <<U8(STX)
        <<CCmdBase::CmdId()
        <<ver_
        <<result_
        <<U8(ETX);
    out<<Manip::offset_value(from, U16(out.Size() - from));
}

std::string CCmdResp::ToString() const
{
    std::ostringstream oss;
    oss<<"{base="<<CCmdBase::ToString()
        <<", ver_="<<ver_
        <<", result_="<<Tools::Dump(result_)
        <<"}";
    return oss.str();
}

NS_SERVER_END

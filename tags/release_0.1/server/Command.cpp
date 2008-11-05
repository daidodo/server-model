#include <sstream>
#include <common/impl/Alloc.h>
#include <common/impl/Command_impl.h>
#include <common/Logger.h>
#include <common/Tools.h>
#include <common/EncryptorAes.h>
#include "Command.h"

NS_SERVER_BEGIN

//struct ICommand
__DZ_STRING ICommand::CommandName(int cmdtype)
{
    switch(cmdtype){
        //ADD NEW COMMAND HERE
        case CMD_QUERY:
            return "CMD_QUERY";
        case CMD_RESP:
            return "CMD_RESP";
        default:{
            __DZ_OSTRINGSTREAM oss;
            oss<<"INVALID_ID_"<<cmdtype;
            return oss.str();
        }
    }
}

//struct QCmdBase
QCmdBase * QCmdBase::CreateCommand(const __DZ_VECTOR(char) & data,size_t * used)
{
    LOCAL_LOGGER(logger,"QCmdBase::CreateCommand");
    DEBUG("create command from data="<<Tools::DumpHex(data));
    CInByteStream ds(data);
    __DZ_VECTOR(char) decryptData;
    if(0){    //decrypt data if neccessary
        CEncryptorAes aes;
        aes.SetKey(&data[0],ENCRYPT_KEY_LEN);
        int n = aes.Decrypt(data,CMD_TYPE_OFFSET,decryptData);
        if(n < 0){
            ERROR("decrypt cmd data="<<Tools::DumpHex(data)<<" return "<<n);
            return 0;
        }
        DEBUG("decryptData="<<Tools::DumpHex(decryptData));
        ds.SetSource(decryptData);
    }
    //decode cmd
    __CmdType type = 0;
    if(!(ds>>Manip::offset_value(CMD_TYPE_OFFSET,type)))
        return 0;
    QCmdBase * ret = 0;
    switch(U32(type)){
        //ADD NEW COMMAND HERE
        case CMD_QUERY:{
            __CREATE_COMMAND(CQueryCmd);
            break;}
        default:{
            ERROR("create unknown tcp command type="<<U32(type));
        }
    }
    if(ret){
        if(ret->Decode(ds)){
            if(used)
                *used = ds.CurPos();
        }else{
            ERROR("decode cmd error, release it");
            ReleaseCommand(ret);
        }
    }
    return ret;
}

void QCmdBase::ReleaseCommand(QCmdBase *& pCmd)
{
    LOCAL_LOGGER(logger,"QCmdBase::ReleaseCommand");
    if(!pCmd)
        return;
    U32 cmdtype = pCmd->CmdType();
    switch(cmdtype){
        //ADD NEW COMMAND HERE
        case CMD_QUERY:{
            __DELETE_COMMAND(CQueryCmd);
            break;}
        default:
            ERROR("release unknown tcp command type="<<cmdtype
                <<" address="<<pCmd);
    }
    pCmd = 0;
}

//class UdpQCmdBase
UdpQCmdBase * UdpQCmdBase::CreateCommand(const __DZ_VECTOR(char) & data,size_t * used)
{
    LOCAL_LOGGER(logger,"UdpQCmdBase::CreateCommand");
    CInByteStream ds(data);
    __CmdType type = 0;
    if(!(ds>>Manip::offset_value(CMD_TYPE_OFFSET,type)))
        return 0;
    UdpQCmdBase * ret = 0;
    switch(U32(type)){
        //ADD NEW COMMAND HERE
        default:{
            ERROR("create unknown udp command type="<<U32(type));
        }
    }
    if(ret){
        if(ret->Decode(ds)){
            if(used)
                *used = ds.CurPos();
        }else{
            ERROR("decode cmd error, release it");
            ReleaseCommand(ret);
        }
    }
    return ret;
}

void UdpQCmdBase::ReleaseCommand(UdpQCmdBase *& pCmd)
{
    LOCAL_LOGGER(logger,"QCmdBase::ReleaseCommand");
    if(!pCmd)
        return;
    U32 cmdtype = pCmd->CmdType();
    switch(cmdtype){
        //ADD NEW COMMAND HERE
        default:
            ERROR("release unknown udp command type="<<cmdtype
                <<" address="<<pCmd);
    }
    pCmd = 0;
}

//struct CQueryCmd
__DZ_STRING CQueryCmd::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<",fileHash_="<<Tools::DumpHex(fileHash_)
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
        <<",clientHash_="<<Tools::DumpHex(clientHash_)
        <<",peerId_="<<peerId_
        <<")";
    return oss.str();
}

void CQueryRespCmd::EncodeParam(COutByteStream & ds) const
{
    LOCAL_LOGGER(logger,"CQueryRespCmd::EncodeParam");
    DEBUG(ToString());
    ds<<result_<<fileHash_<<clientHash_<<peerId_;
}

NS_SERVER_END

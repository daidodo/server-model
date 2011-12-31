#include <common/impl/Config.h>
#include <sstream>
#include <common/Logger.h>
#include <common/EncryptorAes.h>
#include <common/impl/Command_impl.h>
#include "Command.h"

NS_SERVER_BEGIN

//struct ICommand
std::string ICommand::CommandName(int cmdtype)
{
    switch(cmdtype){
        //ADD NEW COMMAND HERE
        __CMD_CASE(CMD_QUERY);
		__CMD_CASE(CMD_RESP);
        default:{
            std::ostringstream oss;
            oss<<"INVALID_ID_"<<cmdtype;
            return oss.str();
        }
    }
}

//struct QCmdBase
U32 QCmdBase::MaxCmdLength = 1024;

QCmdBase * QCmdBase::CreateCommand(const std::vector<char> & data,size_t * used)
{
    LOCAL_LOGGER(logger,"QCmdBase::CreateCommand");
    DEBUG("create command from data="<<Tools::DumpHex(data));
    CInByteStream ds(data);
    U16 ver = 0;
    ds>>Manip::offset_value(0,ver);
	std::vector<char> decryptData;
    if(ver > 100){    //decrypt data if neccessary
        CEncryptorAes aes;
        aes.SetKey(&data[0],ENCRYPT_KEY_LEN);
        int n = aes.Decrypt(data,decryptData,HEAD_LEN);
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
        __CREATE_CMD_CASE(CMD_QUERY,CQueryCmd);
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
        __DELETE_CMD_CASE(CMD_QUERY,CQueryCmd);
        default:
            ERROR("release unknown tcp command type="<<cmdtype
                <<" address="<<pCmd);
    }
    pCmd = 0;
}

QCmdBase::QCmdBase(U32 cmdtype)
    : version_(CMD_VERSION)
    , cmdtype_(cmdtype)
    , seq_(0)
    , length_(0)
    , useHttp_(false)
{}

bool QCmdBase::Decode(CInByteStream & ds)
{
    return QCmdBase::DecodeParam(ds) && DecodeParam(ds);
}

std::string QCmdBase::ToString() const
{
    return std::string("(") + QCmdBase::ToStringHelp() + ToStringHelp() + std::string(")");
}

std::string QCmdBase::ToStringHelp() const
{
    std::ostringstream oss;
    oss<<(useHttp_ ? "HTTP" : "")
        <<"(version_="<<std::dec<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<",length_="<<length_
        <<")";
    return oss.str();
}

QCmdBase::QCmdBase(U32 cmdtype,const QCmdBase & qhead)
    : version_(qhead.version_)
    , cmdtype_(cmdtype)
    , seq_(qhead.seq_)
    , length_(0)
    , useHttp_(qhead.useHttp_)
{}

bool QCmdBase::DecodeParam(CInByteStream & ds)
{
    return ((ds>>version_>>cmdtype_>>seq_>>length_)
	    && version_ >= CMD_VERSION
		&& length_ <= MaxCmdLength);
}

//struct RCmdBase
RCmdBase::RCmdBase(U32 cmdtype)
    : QCmdBase(cmdtype)
{}

RCmdBase::RCmdBase(U32 cmdtype,const QCmdBase & qhead)
    : QCmdBase(cmdtype,qhead)
{}

void RCmdBase::Encode(COutByteStream & ds) const
{
    size_t start = ds.Size();
    if(version_ > 100){  //encrypt data if neccessary
        COutByteStream dss;
        RCmdBase::EncodeParam(dss);
        EncodeParam(dss);
        std::vector<char> data,encryptData;
        dss.ExportData(data);
        CEncryptorAes aes;
        aes.SetKey(&data[0],QCmdBase::ENCRYPT_KEY_LEN);
        aes.Encrypt(data,encryptData,QCmdBase::HEAD_LEN);
        ds<<Manip::raw(&encryptData[0],encryptData.size());
    }else{
        RCmdBase::EncodeParam(ds);
        EncodeParam(ds);
    }
    size_t len = ds.Size() - start;
    ds<<Manip::offset_value(start + LEN_OFFSET,U32(len - HEAD_LEN));
}

void RCmdBase::EncodeParam(COutByteStream & ds) const
{
    ds<<version_<<cmdtype_<<seq_<<length_;
}

//struct UdpQCmdBase
U32 UdpQCmdBase::MaxCmdLength = 1024;

UdpQCmdBase * UdpQCmdBase::CreateCommand(const std::vector<char> & data,size_t * used)
{
    LOCAL_LOGGER(logger,"UdpQCmdBase::CreateCommand");
    std::vector<char> decryptData;
    CInByteStream ds(data);
    if(0){    //decrypt data if neccessary
        CEncryptorAes aes;
        aes.SetKey(&data[0],ENCRYPT_KEY_LEN);
        int n = aes.Decrypt(data,decryptData,HEAD_LEN);
        if(n < 0){
            ERROR("decrypt cmd data="<<Tools::DumpHex(data)<<" return "<<n);
            return 0;
        }
        DEBUG("decryptData="<<Tools::DumpHex(decryptData));
        ds.SetSource(decryptData);
    }
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

UdpQCmdBase::UdpQCmdBase(U32 cmdtype)
    : version_(CMD_VERSION)
    , cmdtype_(cmdtype)
    , seq_(0)
{}

bool UdpQCmdBase::Decode(CInByteStream & ds)
{
    return UdpQCmdBase::DecodeParam(ds) && DecodeParam(ds);
}

std::string UdpQCmdBase::ToString() const
{
    return std::string("(") + UdpQCmdBase::ToStringHelp() + ToStringHelp() + std::string(")");
}

std::string UdpQCmdBase::ToStringHelp() const
{
    std::ostringstream oss;
    oss<<"(version_="<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<")";
    return oss.str();
}

UdpQCmdBase::UdpQCmdBase(U32 cmdtype,const UdpQCmdBase & qhead)
    : version_(qhead.version_)
    , cmdtype_(cmdtype)
    , seq_(time(0))
{}

bool UdpQCmdBase::DecodeParam(CInByteStream & ds)
{
    return ((ds>>version_>>cmdtype_>>seq_)
	    && version_ <= CMD_VERSION);
}

//struct UdpRCmdBase
UdpRCmdBase::UdpRCmdBase(U32 cmdtype)
    : UdpQCmdBase(cmdtype)
{}

UdpRCmdBase::UdpRCmdBase(U32 cmdtype,const UdpQCmdBase & qhead)
    : UdpQCmdBase(cmdtype,qhead)
{}

void UdpRCmdBase::Encode(COutByteStream & ds) const
{
    if(0){  //encrypt data if neccessary
        COutByteStream dss;
        UdpRCmdBase::EncodeParam(dss);
        EncodeParam(dss);
        std::vector<char> data,encryptData;
        dss.ExportData(data);
        CEncryptorAes aes;
        aes.SetKey(&data[0],UdpQCmdBase::ENCRYPT_KEY_LEN);
        aes.Encrypt(data,encryptData,UdpQCmdBase::HEAD_LEN);
        ds<<Manip::raw(&encryptData[0],encryptData.size());
    }else{
        UdpRCmdBase::EncodeParam(ds);
        EncodeParam(ds);
    }
}

void UdpRCmdBase::EncodeParam(COutByteStream & ds) const
{
    ds<<version_<<cmdtype_<<seq_;
}

NS_SERVER_END

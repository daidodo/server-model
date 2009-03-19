#include <sstream>
#include <common/EncryptorAes.h>
#include "CommandBase.h"

NS_SERVER_BEGIN

//struct QCmdBase
QCmdBase::QCmdBase()
    : useHttp_(false)
{}

bool QCmdBase::Decode(CInByteStream & ds)
{
    return QCmdBase::DecodeParam(ds) && DecodeParam(ds);
}

__DZ_STRING QCmdBase::ToString() const
{
    return __DZ_STRING("(") + QCmdBase::ToStringHelp() + ToStringHelp() + __DZ_STRING(")");
}

__DZ_STRING QCmdBase::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<(useHttp_ ? "HTTP" : "")
        <<"(version_="<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<",length_="<<length_
        <<")";
    return oss.str();
}

bool QCmdBase::DecodeParam(CInByteStream & ds)
{
    return (ds>>version_>>cmdtype_>>seq_>>length_);
}

//struct RCmdBase
RCmdBase::RCmdBase(U32 cmdtype)
    : version_(CMD_VERSION)
    , cmdtype_(cmdtype)
    , seq_(0)
    , length_(0)
    , useHttp_(false)
{}

RCmdBase::RCmdBase(U32 cmdtype,const QCmdBase & qhead)
    : version_(qhead.version_)
    , cmdtype_(cmdtype)
    , seq_(qhead.seq_)
    , length_(0)
    , useHttp_(qhead.useHttp_)
{}

void RCmdBase::Encode(COutByteStream & ds) const
{
    size_t start = ds.Size();
    if(version_ > 100){  //encrypt data if neccessary
        COutByteStream dss;
        RCmdBase::EncodeParam(dss);
        EncodeParam(dss);
        __DZ_VECTOR(char) data,encryptData;
        dss.ExportData(data);
        CEncryptorAes aes;
        aes.SetKey(&data[0],QCmdBase::ENCRYPT_KEY_LEN);
        aes.Encrypt(data,QCmdBase::HEAD_LEN,encryptData);
        ds<<Manip::raw(&encryptData[0],encryptData.size());
    }else{
        RCmdBase::EncodeParam(ds);
        EncodeParam(ds);
    }
    size_t len = ds.Size() - start;
    ds<<Manip::offset_value(start + LEN_OFFSET,U32(len - HEAD_LEN));
}

__DZ_STRING RCmdBase::ToString() const
{
    return __DZ_STRING("(") + RCmdBase::ToStringHelp() + ToStringHelp() + __DZ_STRING(")");
}

__DZ_STRING RCmdBase::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<(useHttp_ ? "HTTP" : "")
        <<"(version_="<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<",length_="<<length_
        <<")";
    return oss.str();
}

void RCmdBase::EncodeParam(COutByteStream & ds) const
{
    ds<<version_<<cmdtype_<<seq_<<length_;
}

//struct UdpQCmdBase
bool UdpQCmdBase::Decode(CInByteStream & ds)
{
    return UdpQCmdBase::DecodeParam(ds) && DecodeParam(ds);
}

__DZ_STRING UdpQCmdBase::ToString() const
{
    return __DZ_STRING("(") + UdpQCmdBase::ToStringHelp() + ToStringHelp() + __DZ_STRING(")");
}

__DZ_STRING UdpQCmdBase::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<"(version_="<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<")";
    return oss.str();
}

bool UdpQCmdBase::DecodeParam(CInByteStream & ds)
{
    return (ds>>version_>>cmdtype_>>seq_);
}

//struct UdpRCmdBase
UdpRCmdBase::UdpRCmdBase(U32 cmdtype)
    : version_(CMD_VERSION)
    , cmdtype_(cmdtype)
    , seq_(0)
{}

UdpRCmdBase::UdpRCmdBase(U32 cmdtype,const UdpQCmdBase & qhead)
    : version_(qhead.version_)
    , cmdtype_(cmdtype)
    , seq_(qhead.seq_)
{}

void UdpRCmdBase::Encode(COutByteStream & ds) const
{
    UdpRCmdBase::EncodeParam(ds);
    EncodeParam(ds);
}

__DZ_STRING UdpRCmdBase::ToString() const
{
    return __DZ_STRING("(") + UdpRCmdBase::ToStringHelp() + ToStringHelp() + __DZ_STRING(")");
}

__DZ_STRING UdpRCmdBase::ToStringHelp() const
{
    __DZ_OSTRINGSTREAM oss;
    oss<<"(version_="<<version_
        <<","<<CommandName(cmdtype_)
        <<",seq_="<<seq_
        <<")";
    return oss.str();
}

void UdpRCmdBase::EncodeParam(COutByteStream & ds) const
{
    ds<<version_<<cmdtype_<<seq_;
}

NS_SERVER_END

#ifndef DOZERG_COMMAND_BASE_H_20080103
#define DOZERG_COMMAND_BASE_H_20080103

#include <common/DataStream.h>

NS_SERVER_BEGIN

//Interface for Cmd class
struct ICommand
{
    static const U32 CMD_VERSION = 1;
    static __DZ_STRING CommandName(int cmdtype);
    virtual __DZ_STRING ToStringHelp() const = 0;
    virtual bool DecodeParam(CInByteStream & ds) = 0;
    virtual void EncodeParam(COutByteStream & ds) const = 0;
    virtual ~ICommand(){}
};

//TCP Query Cmd base
class QCmdBase : public ICommand
{
protected:
    typedef U16 __CmdType;
    U16         version_;
    __CmdType   cmdtype_;
    U32         seq_;
    U32         length_;
    //命令是否添加HTTP头传送
    bool        useHttp_;
public:
    static const size_t HEAD_LEN = 12;          //bytes, MUST be consist with the QCmdBase definition
    static const size_t CMD_TYPE_OFFSET = 2;    //bytes, MUST be consist with the QCmdBase definition
    static const size_t ENCRYPT_KEY_LEN = 8;    //bytes, MUST be consist with the Document
    static const size_t LEN_OFFSET = 8;         //bytes, MUST be consist with the QCmdBase definition
    static U32 MaxCmdLength;                    //bytes, Max Length of TCP commands
    static QCmdBase * CreateCommand(const __DZ_VECTOR(char) & data,size_t * used = 0);
    static void ReleaseCommand(QCmdBase *& cmd);
    explicit QCmdBase(U32 cmdtype = 0);
    U32 CmdType() const{return U32(cmdtype_);}
    U32 CmdBodyLength() const{return length_;}
    __DZ_STRING ToString() const;
    //virtual functions
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
    void EncodeParam(COutByteStream & ds) const{}
    void UseHttp(bool http){useHttp_ = http;}
    bool UseHttp() const{return useHttp_;}
protected:
    QCmdBase(U32 cmdtype,const QCmdBase & qhead);   //give chances for derived class to init
private:
    bool Decode(CInByteStream & ds);
};

//TCP Response Cmd base
struct RCmdBase : public QCmdBase
{
    explicit RCmdBase(U32 cmdtype);
    RCmdBase(U32 cmdtype,const QCmdBase & qhead);
    U32 Version() const{return version_;}
    void Encode(COutByteStream & ds) const;
    //virtual functions
    bool DecodeParam(CInByteStream & ds){return true;}
    void EncodeParam(COutByteStream & ds) const;
};

//UDP Query Cmd base
class UdpQCmdBase : public ICommand
{
protected:
    typedef U16 __CmdType;
    U16         version_;
    __CmdType   cmdtype_;
    U32         seq_;
public:
    static const size_t HEAD_LEN = 8;          //bytes, MUST be consist with the UdpQCmdBase definition
    static const size_t CMD_TYPE_OFFSET = 2;    //bytes, MUST be consist with the UdpQCmdBase definition
    static const size_t ENCRYPT_KEY_LEN = 8;    //bytes, MUST be consist with the Document
    static U32 MaxCmdLength;                    //bytes, Max Length of UDP commands
    static UdpQCmdBase * CreateCommand(const __DZ_VECTOR(char) & data,size_t * used = 0);
    static void ReleaseCommand(UdpQCmdBase *& cmd);
    explicit UdpQCmdBase(U32 cmdtype = 0);
    U32 CmdType() const{return U32(cmdtype_);}
    __DZ_STRING ToString() const;
    //virtual functions
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
    void EncodeParam(COutByteStream & ds) const{}
protected:
    UdpQCmdBase(U32 cmdtype,const UdpQCmdBase & qhead); //give chances for derived class to init
private:
    bool Decode(CInByteStream & ds);
};

//UDP Response Cmd base
struct UdpRCmdBase : public UdpQCmdBase
{
    explicit UdpRCmdBase(U32 cmdtype);
    UdpRCmdBase(U32 cmdtype,const UdpQCmdBase & qhead);
    void Encode(COutByteStream & ds) const;
    //virtual functions
    bool DecodeParam(CInByteStream & ds){return true;}
    void EncodeParam(COutByteStream & ds) const;
};

NS_SERVER_END

#endif

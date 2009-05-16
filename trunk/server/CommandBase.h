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

class RCmdBase;

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
    static const size_t LEN_OFFSET = 8;        //bytes, MUST be consist with the RCmdBase definition
    static QCmdBase * CreateCommand(const __DZ_VECTOR(char) & data,size_t * used = 0);
    static void ReleaseCommand(QCmdBase *& cmd);
    QCmdBase();
    U32 CmdType() const{return U32(cmdtype_);}
    U32 CmdBodyLength() const{return length_;}
    virtual __DZ_STRING ToString() const;
    //virtual functions
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
    void EncodeParam(COutByteStream & ds) const{}
    void UseHttp(bool http){useHttp_ = http;}
    bool UseHttp() const{return useHttp_;}
protected:
    explicit QCmdBase(U32 cmdtype); //give chance for derived class to init
    QCmdBase(U32 cmdtype,const QCmdBase & qhead);
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
    __DZ_STRING ToString() const;
    //virtual functions
    bool DecodeParam(CInByteStream & ds){return true;}
    void EncodeParam(COutByteStream & ds) const;
};

class UdpRCmdBase;

//UDP Query Cmd base
class UdpQCmdBase : public ICommand
{
    friend class UdpRCmdBase;
    typedef U16 __CmdType;
    U16         version_;
    __CmdType   cmdtype_;
    U32         seq_;
public:
    static const size_t CMD_TYPE_OFFSET = 2;   //bytes, MUST be consist with the UdpQCmdBase definition
    static UdpQCmdBase * CreateCommand(const __DZ_VECTOR(char) & data,size_t * used = 0);
    static void ReleaseCommand(UdpQCmdBase *& cmd);
    U32 CmdType() const{return U32(cmdtype_);}
    __DZ_STRING ToString() const;
    //virtual functions
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
    void EncodeParam(COutByteStream & ds) const{}
private:
    bool Decode(CInByteStream & ds);
};

class UdpRCmdBase : public ICommand
{
    U16 version_;
    U16 cmdtype_;
    U32 seq_;
public:
    explicit UdpRCmdBase(U32 cmdtype);
    UdpRCmdBase(U32 cmdtype,const UdpQCmdBase & qhead);
    U32 CmdType() const{return cmdtype_;}
    void Encode(COutByteStream & ds) const;
    __DZ_STRING ToString() const;
    //virtual functions
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds){return true;}
    void EncodeParam(COutByteStream & ds) const;
};

NS_SERVER_END

#endif

#ifndef DOZERG_CMD_BASE_H_20120122
#define DOZERG_CMD_BASE_H_20120122

#include <vector>
#include <string>

#include <DataStream.h>
#include <RecvHelper.h>

NS_SERVER_BEGIN

const int STX = 3;
const int ETX = 2;

const int CMD_QUERY = 1;
const int CMD_RESP = 2;

class CCmdBase
{
public:
    //从buf中解出cmd
    static CCmdBase * DecodeCmd(const char * buf, size_t sz);
    //释放cmd
    static void ReleaseCmd(CCmdBase * cmd);
    explicit CCmdBase(U16 cmdId)
        : cmdId_(cmdId)
    {}
    virtual ~CCmdBase(){}
    U16 CmdId() const{return cmdId_;}
    virtual std::string ToString() const;
private:
    U16 cmdId_;
};

struct CCmdRecvHelper : public CRecvHelper
{
    //virtual std::string ToString() const{return "CCmdRecvHelper";}
    virtual size_t InitRecvSize() const{return 5;}
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const;
    virtual bool HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const;
    virtual void ReleaseCmd(const CAnyPtr & cmd) const;
    virtual __Events ProcessCmd(const CAnyPtr & cmd, CSockHanle & handle) const;
};

struct CCmdQuery : public CCmdBase
{
    CCmdQuery()
        : CCmdBase(CMD_QUERY)
    {}
    U16 Ver() const{return ver_;}
    bool Decode(CInByteStream & in);
    std::string ToString() const;
private:
    U16 ver_;
    std::string echo_;
};

struct CCmdResp : public CCmdBase
{
    explicit CCmdResp(const CCmdQuery & query)
        : CCmdBase(CMD_RESP)
        , ver_(query.Ver())
    {}
    void Result(){result_ = "this is result";}
    void Encode(COutByteStream & out) const;
    std::string ToString() const;
private:
    U16 ver_;
    std::string result_;
};

NS_SERVER_END

#endif


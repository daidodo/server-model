#ifndef DOZERG_CMD_BASE_H_20120122
#define DOZERG_CMD_BASE_H_20120122

#include <vector>
#include <string>

#include <DataStream.h>
#include "SockSession.h"

NS_SERVER_BEGIN

class CCmdBase
{
    static const int STX = 3;
    static const int ETX = 2;
protected:
    static const int CMD_QUERY = 1;
    static const int CMD_RESP = 2;
public:
    static __OnDataArriveRet OnDataArrive(const char * buf, size_t sz);
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

typedef CCmdBase __CmdBase;

struct CCmdQuery : public CCmdBase
{
    CCmdQuery()
        : CCmdBase(CMD_QUERY)
    {}
    bool Decode(CInByteStream & in);
    std::string ToString() const;
private:
    U16 ver_;
    std::string echo_;
};

struct CCmdResp : public CCmdBase
{
    CCmdResp()
        : CCmdBase(CMD_RESP)
    {}
    std::string ToString() const;
private:
    U16 ver_;
    std::string result_;
};

NS_SERVER_END

#endif


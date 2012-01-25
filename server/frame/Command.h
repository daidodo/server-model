#ifndef DOZERG_CMD_BASE_H_20120122
#define DOZERG_CMD_BASE_H_20120122

#include <vector>
#include <string>

#include <impl/Config.h>

NS_SERVER_BEGIN

typedef std::vector<char> __Buffer;

struct CCmdBase
{
    static const int CMD_QUERY = 1;
    static const int CMD_RESP = 2;
    virtual ~CCmdBase() = 0;
    std::string ToString() const{return "";}
private:
    U16 cmdId_;
};

//从buf中解出cmd
CCmdBase * DecodeCmd(const __Buffer & buf);

//释放cmd
void ReleaseCmd(CCmdBase * cmd) __DZ_NOTHROW;

typedef CCmdBase __CmdBase;

struct CCmdQuery : public CCmdBase
{
    U16 ver_;
    std::string echo_;
};

struct CCmdResp : public CCmdBase
{
    U16 ver_;
    std::string result_;
};

NS_SERVER_END

#endif


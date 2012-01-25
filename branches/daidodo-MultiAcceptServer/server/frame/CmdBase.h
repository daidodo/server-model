#ifndef DOZERG_CMD_BASE_H_20120122
#define DOZERG_CMD_BASE_H_20120122

#include <vector>
#include <string>

#include <impl/Config.h>

NS_SERVER_BEGIN

typedef std::vector<char> __Buffer;

struct CCmdBase
{
    std::string ToString() const{return "";}
};

//从buf中解出cmd
CCmdBase * DecodeCmd(const __Buffer & buf);

//释放cmd
void ReleaseCmd(CCmdBase * cmd) __DZ_NOTHROW;

typedef CCmdBase __CmdBase;

NS_SERVER_END

#endif


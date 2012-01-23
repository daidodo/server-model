#ifndef DOZERG_CMD_BASE_H_20120122
#define DOZERG_CMD_BASE_H_20120122

#include <impl/Config.h>

NS_SERVER_BEGIN

struct CCmdBase
{
    static CCmdBase * GetObject(){return 0;}
    std::string ToString() const{return "";}
};

typedef CCmdBase __CmdBase;

NS_SERVER_END

#endif


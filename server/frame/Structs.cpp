#include "Structs.h"

NS_SERVER_BEGIN

//struct CCmdSock

bool CCmdSock::RecvCmd(__CmdBase *& cmd)
{
    cmd = 0;
    return true;
}


NS_SERVER_END

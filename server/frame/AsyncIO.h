#ifndef DOZERG_ASYNC_IO_H_20120117
#define DOZERG_ASYNC_IO_H_20120117

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

class CHahsEngine;

struct CAsyncIO : public CThreadPool
{
    CAsyncIO(size_t stackSz, CHahsEngine & engine);
    bool Init(){return true;}
protected:
    virtual int doIt();
private:
    bool handleOutput(__SockPtr & sock);
    bool handleInput(__SockPtr & sock, __FdList & addingList, __QueryCmdList & queryCmdList);
    bool handleAccept(__SockPtr & sock, __FdList & addingList);
    bool handleRecv(__SockPtr & sock, __QueryCmdList & queryCmdList, bool isUdp);
    bool handleCmd(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr, __QueryCmdList & queryCmdList);
    //members
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

NS_SERVER_END

#endif


#ifndef DOZERG_ASYNC_IO_H_20120117
#define DOZERG_ASYNC_IO_H_20120117

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

struct CAsyncIoCtorParams
{
    size_t stackSize_;
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

struct CAsyncIoInitParams
{
};

struct CAsyncIO : public CThreadPool
{
    explicit CAsyncIO(const CAsyncIoCtorParams & params);
    bool Init(const CAsyncIoInitParams & params){return true;}
protected:
    virtual int doIt();
private:
    bool handleOutput(__SockPtr & sock);
    bool handleInput(__SockPtr & sock, __FdList & addingList);
    bool handleAccept(__SockPtr & sock, __FdList & addingList);
    bool handleCmd(__SockPtr & sock, __CmdBase * cmd);
    //members
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

NS_SERVER_END

#endif


#ifndef DOZERG_ASYNC_IO_H_20120117
#define DOZERG_ASYNC_IO_H_20120117

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

struct CAsyncIoCtorParams
{
    size_t stackSize_;
    __FdEventQue & addingQue_;
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
    bool handleSend(__SockPtr & sock, __FdEventList & addingList);
    bool handleRecv(__SockPtr & sock, __FdEventList & addingList);
    bool handleAccept(__SockPtr & sock, __FdEventList & addingList);
    bool handleCmd(__SockPtr & sock, CCmdBase * cmd);
    //members
    __FdEventQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

NS_SERVER_END

#endif


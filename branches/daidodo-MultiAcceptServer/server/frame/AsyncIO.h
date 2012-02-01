#ifndef DOZERG_ASYNC_IO_H_20120117
#define DOZERG_ASYNC_IO_H_20120117

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

class CHahsEngine;

class CAsyncIO : public CThreadPool
{
    struct CListParams{
        __SockPtrList sockList_;
        __FdEventList eventList_;
        __FdList addingList_;
        __QueryCmdList queryCmdList_;
    };
public:
    CAsyncIO(size_t stackSz, CHahsEngine & engine);
    bool Init(){return true;}
protected:
    virtual int doIt();
private:
    bool handleOutput(__SockPtr & sock);
    bool handleInput(__SockPtr & sock, CListParams & listParams);
    bool handleAccept(__SockPtr & sock, CListParams & listParams);
    bool handleRecv(__SockPtr & sock, CListParams & listParams, bool isUdp);
    bool handleCmd(__SockPtr & sock, CCmdBase * cmd, CSockAddr & udpClientAddr, CListParams & listParams);
    //members
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

NS_SERVER_END

#endif


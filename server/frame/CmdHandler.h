#ifndef DOZERG_CMD_HANDLER_H_20120118
#define DOZERG_CMD_HANDLER_H_20120118

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

struct CHandlerCtorParams
{
    size_t stackSize_;
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
    __QueryCmdQue & queryCmdQue_;
};

struct CHandlerInitParams
{
    int threadCountMax_;
};

class CCmdHandler : public CThreadManager<__QueryCmdQue>
{
    typedef CThreadManager<__QueryCmdQue> __MyBase;
    explicit CCmdHandler(const CHandlerCtorParams & params);
    bool Init(const CHandlerInitParams & params);
protected:
    virtual void doIt(__Job & job);
private:
    //members
    __FdQue & addingQue_;
    __FdEventQue & eventQue_;
    __FdSockMap & fdSockMap_;
};

NS_SERVER_END

#endif


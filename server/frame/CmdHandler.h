#ifndef DOZERG_CMD_HANDLER_H_20120118
#define DOZERG_CMD_HANDLER_H_20120118

#include <Threads.h>

#include "Structs.h"

NS_SERVER_BEGIN

class CHahsEngine;

class CCmdHandler : public CThreadManager<__QueryCmdQue>
{
    typedef CThreadManager<__QueryCmdQue> __MyBase;
public:
    CCmdHandler(size_t stackSz, CHahsEngine & engine);
    bool Init(int threadCountMax);
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


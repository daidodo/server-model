#include "CmdHandler.h"

NS_SERVER_BEGIN

CCmdHandler::CCmdHandler(const CHandlerCtorParams & params)
    : __MyBase(params.queryCmdQue_, params.stackSize_)
    , addingQue_(params.addingQue_)
    , eventQue_(params.eventQue_)
    , fdSockMap_(params.fdSockMap_)
{}

bool CCmdHandler::Init(const CHandlerInitParams & params)
{
    __MyBase::ThreadCountMax(params.threadCountMax_);
    return true;
}

void CCmdHandler::doIt(__Job & job)
{
    
}


NS_SERVER_END

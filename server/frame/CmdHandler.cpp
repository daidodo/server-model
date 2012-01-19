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
    typedef CSharedPtr<__CmdBase, false> __CmdBasePtr;
    LOCAL_LOGGER(logger, "CCmdHandler::doIt");
    __CmdBasePtr cmd(job.first);    //guard
    const int fd = job.second;
    if(!cmd || fd < 0){
        ERROR("fd="<<fd<<" or cmd="<<Tools::ToStringPtr(cmd)<<" is invalid");
        return;
    }
    __SockPtr sock;
    fdSockMap_.GetSock(fd, sock);
    if(!sock || sock != job.third){
        ERROR("before process cmd="<<Tools::ToStringPtr(cmd)<<" for fd="<<fd<<", old sock="<<Tools::ToStringPtr(job.third)<<" is replaced by new sock="<<Tools::ToStringPtr(sock)<<", abandon it");
        return;
    }
    __Events oldEv = sock->Events();
    sock->Process(*cmd);
    if(oldEv != sock->Events()){
        if(!addingQue_.Push(fd, 200)){
            ERROR("addingQue_.Push(fd="<<fd<<") failed for cmd="<<Tools::ToStringPtr(cmd)<<" from sock="<<Tools::ToStringPtr(sock));
        }
    }
}


NS_SERVER_END

#include "Command.h"
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
    typedef CSharedPtr<__CmdSession, false> __CmdSessionPtr;
    LOCAL_LOGGER(logger, "CCmdHandler::doIt");
    __CmdSessionPtr session(job);    //guard
    __SockPtr sock = session->SockPtr();
    __CmdBase * cmd = session->CmdBase();
    if(!cmd || !sock->IsValid()){
        ERROR("cmd="<<Tools::ToStringPtr(cmd)<<" or sock="<<Tools::ToStringPtr(sock)<<" is invalid");
        return;
    }
    const int fd = sock->Fd();
    __SockPtr newSock;
    fdSockMap_.GetSock(fd, newSock);
    if(!newSock || sock != newSock){
        ERROR("before process session="<<Tools::ToStringPtr(session)<<", old sock is replaced by new sock="<<Tools::ToStringPtr(newSock)<<", abandon it");
        return;
    }
    __Events oldEv = sock->Events();
    sock->Process(*cmd, session->UdpClientAddr());
    if(oldEv != sock->Events()){
        if(!addingQue_.Push(fd, 200)){
            ERROR("addingQue_.Push(fd="<<fd<<") failed for session="<<Tools::ToStringPtr(session));
        }
    }
}

NS_SERVER_END

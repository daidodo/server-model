#include <Logger.h>

#include "SockSession.h"

NS_SERVER_BEGIN

//struct CSockSession

CSockSession::CSockSession(const CRecvHelper & recvHelper)
    : fileDesc_(0)
    , recvHelper_(recvHelper)
    , needSz_(0)
    , stepIndex_(0)
    , ev_(0)
{}

CSockSession::~CSockSession()
{
    if(fileDesc_){
        IFileDesc::PutObject(fileDesc_);
        fileDesc_ = 0;
    }
}

bool CSockSession::Accept(CSockSession *& client)
{
    typedef CSharedPtr<CTcpConnSocket, false> __TcpConnPtr;
    LOCAL_LOGGER(logger, "CSockSession::Accept");
    assert(IsValid());
    client = 0;
    CListenSocket * listen = dynamic_cast<CListenSocket *>(fileDesc_);
    if(!listen){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CListenSocket");
        return false;
    }
    __TcpConnPtr clientSock(dynamic_cast<CTcpConnSocket *>(IFileDesc::GetObject(FD_TCP_CONN)));
    if(!clientSock)
        return false;
    int ret = listen->Accept(*clientSock);
    if(CListenSocket::RET_ERROR == ret){
        ERROR("listen sock="<<Tools::ToStringPtr(listen)<<" accept client error"<<IFileDesc::ErrMsg());
        return false;
    }else if(CListenSocket::RET_EAGAIN == ret)
        return true;
    clientSock->SetLinger();
    clientSock->SetBlock(false);
    client = GetObject(recvHelper_);
    if(!client){
        ERROR("no memory for sock session of clientSock="<<Tools::ToStringPtr(clientSock)<<", close it");
        return false;
    }
    client->ev_ = EVENT_TCP_RECV;
    client->fileDesc_ = &*clientSock;
    clientSock.release();
    return true;
}

bool CSockSession::RecvCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr)
{
    if(Events::CanTcpRecv(ev_)){
        return recvTcpCmd(cmd);
    }else if(Events::CanUdpRecv(ev_)){
        return recvUdpCmd(cmd, udpClientAddr);
    }
    LOCAL_LOGGER(logger, "CSockSession::RecvCmd");
    ERROR("invalid ev_="<<ev_<<" for recv sock="<<ToString());
    return false;
}

bool CSockSession::recvTcpCmd(__CmdBase *& cmd)
{
    LOCAL_LOGGER(logger, "CSockSession::recvTcpCmd");
    assert(IsValid());
    cmd = 0;
    CTcpConnSocket * conn = dynamic_cast<CTcpConnSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" for CTcpConnSocket");
        return false;
    }

    return true;
}

bool CSockSession::recvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::recvUdpCmd");
    assert(IsValid());
    cmd = 0;
    return true;
}

NS_SERVER_END

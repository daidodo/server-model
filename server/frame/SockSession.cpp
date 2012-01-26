#include <Logger.h>
#include <DataStream.h>

#include "Command.h"
#include "SockSession.h"

NS_SERVER_BEGIN

//struct CSockSession

CSockSession::CSockSession(IFileDesc * fileDesc, const CRecvHelper & recvHelper)
    : fileDesc_(fileDesc_)
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
    if(!clientSock){
        ERROR("cannot get CTcpConnSocket object for sock="<<ToString());
        return false;
    }
    int ret = listen->Accept(*clientSock);
    if(CListenSocket::RET_ERROR == ret){
        ERROR("listen sock="<<Tools::ToStringPtr(listen)<<" accept client error"<<IFileDesc::ErrMsg());
        return false;
    }else if(CListenSocket::RET_EAGAIN == ret){
        ev_ |= EVENT_ACCEPT;
        return true;
    }
    clientSock->SetLinger();
    clientSock->SetBlock(false);
    client = GetObject(&*clientSock, recvHelper_);
    if(!client){
        ERROR("no memory for sock session of clientSock="<<Tools::ToStringPtr(clientSock)<<", close it");
        return false;
    }
    client->ev_ = EVENT_TCP_RECV;
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

void CSockSession::ReleaseCmd(__CmdBase * cmd)
{
    __ReleaseCmd releaseCmd = recvHelper_.ReleaseCmd();
    assert(releaseCmd);
    releaseCmd(cmd);
}

bool CSockSession::AddOutBuf(__Buffer & buf, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::AddOutBuf");
    assert(IsValid());
    outList_.resize(outList_.size() + 1);
    outList_.back().swap(buf);
    if(FD_UDP == fileDesc_->Type()){
        if(!udpClientAddr.IsValid()){
            ERROR("invalid udpClientAddr="<<udpClientAddr.ToString()<<" for sock="<<ToString());
            return false;
        }
        addrList_.resize(addrList_.size() + 1);
        addrList_.back().Swap(udpClientAddr);
    }
    return true;
}

bool CSockSession::SendBuffer()
{
    if(Events::CanTcpSend(ev_)){
        return tcpSend();
    }else if(Events::CanUdpSend(ev_)){
        return udpSend();
    }
    LOCAL_LOGGER(logger, "CSockSession::SendBuffer");
    ERROR("invalid ev_="<<ev_<<" for send sock="<<ToString());
    return false;
}

bool CSockSession::WriteData()
{
    LOCAL_LOGGER(logger, "CSockSession::WriteData");
    assert(IsValid());
    CFile * file = dynamic_cast<CFile *>(fileDesc_);
    if(!file){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CFile");
        return false;
    }
    for(__BufList::iterator i = outList_.begin();i != outList_.end();){
        __Buffer & buf = *i;
        if(!buf.empty()){
            ssize_t sz = file->Write(buf);
            if(sz < 0){
                if(EAGAIN == errno || EWOULDBLOCK == errno){
                    ev_ |= EVENT_WRITE;
                    break;
                }
                ERROR("Write failed for buf="<<Tools::Dump(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
                return false;
            }else if(size_t(sz) < buf.size()){
                buf.erase(buf.begin(), buf.begin() + sz);
                ev_ |= EVENT_WRITE;
                break;
            }
        }
        outList_.erase(i++);
    }
    return true;
}

void CSockSession::Process(__CmdBase & cmd, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::Process");
    CCmdQuery & query = dynamic_cast<CCmdQuery &>(cmd);
    INFO("process query="<<query.ToString()<<", udpClientAddr="<<udpClientAddr.ToString());
    CCmdResp resp(query);
    resp.Result();
    INFO("resp="<<resp.ToString()<<" for query="<<query.ToString());
    COutByteStream out;
    resp.Encode(out);
    __Buffer buf;
    out.ExportData(buf);
    AddOutBuf(buf, udpClientAddr);
}

bool CSockSession::recvTcpCmd(__CmdBase *& cmd)
{
    LOCAL_LOGGER(logger, "CSockSession::recvTcpCmd");
    assert(IsValid());
    cmd = 0;
    if(!recvHelper_.IsTcpValid()){
        ERROR("recvHelper_ is invalid for tcp sock="<<ToString());
        return false;
    }
    CTcpConnSocket * conn = dynamic_cast<CTcpConnSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CTcpConnSocket");
        return false;
    }
    if(!needSz_)
        needSz_ = recvHelper_.InitRecvSize();
    for(;;){
        if(!needSz_){
            ERROR("needSz_ is invalid for sock="<<ToString());
            return false;
        }
        ssize_t sz = conn->RecvData(recvBuf_, needSz_);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                ev_ |= EVENT_TCP_RECV;
                break;
            }
            ERROR("RecvData(needSz_="<<needSz_<<") failed for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else if(sz == 0){
            WARN("client closed for sock="<<ToString());
            return false;
        }else if(size_t(sz) < needSz_){
            needSz_ -= sz;
            ev_ |= EVENT_TCP_RECV;
            break;
        }else{
            __OnDataArrive onArrive = recvHelper_.OnDataArrive();
            assert(onArrive);
            const __OnDataArriveRet ret = onArrive(&recvBuf_[0], recvBuf_.size());
            switch(ret.first){
                case RR_COMPLETE:
                    return decodeCmd(cmd, ret.second);
                case RR_NEED_MORE:
                    needSz_ = ret.second;
                    break;
                case RR_ERROR:
                    ERROR("onArrive() failed for sock="<<ToString());
                    return false;
            }
        }
    }
    return true;
}

bool CSockSession::recvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::recvUdpCmd");
    assert(IsValid());
    cmd = 0;
    if(!recvHelper_.IsUdpValid()){
        ERROR("recvHelper_ is invalid for udp sock="<<ToString());
        return false;
    }
    CUdpSocket * conn = dynamic_cast<CUdpSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CUdpSocket");
        return false;
    }
    if(!needSz_)
        needSz_ = recvHelper_.InitRecvSize();
    if(!needSz_)
        needSz_ = 4096;     //default 4K
    for(;;){
        ssize_t sz = conn->RecvData(udpClientAddr, recvBuf_, needSz_);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                ev_ |= EVENT_UDP_RECV;
                break;
            }
            ERROR("RecvData(needSz_="<<needSz_<<") failed for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else{
            __OnDataArrive onArrive = recvHelper_.OnDataArrive();
            if(onArrive){
                const __OnDataArriveRet ret = onArrive(&recvBuf_[0], recvBuf_.size());
                if(ret.first == RR_ERROR){
                    ERROR("onArrive() failed for sock="<<ToString());
                    return false;
                }
            }
            return decodeCmd(cmd, 0);
        }
    }
    return true;
}

bool CSockSession::decodeCmd(__CmdBase *& cmd, size_t left)
{
    LOCAL_LOGGER(logger, "CSockSession::decodeCmd");
    __DecodeCmd decodeCmd = recvHelper_.DecodeCmd();
    assert(decodeCmd && left < recvBuf_.size());
    size_t len = recvBuf_.size() - left;
    cmd = decodeCmd(&recvBuf_[0], len);
    if(!cmd){
        ERROR("DecodeCmd failed for recvBuf_="<<Tools::Dump(recvBuf_)<<", left="<<left<<" for sock="<<ToString());
        return false;
    }
    if(left)
        recvBuf_.erase(recvBuf_.begin(), recvBuf_.begin() + len);
    else
        recvBuf_.clear();
    needSz_ = 0;
    return true;
}

bool CSockSession::tcpSend()
{
    LOCAL_LOGGER(logger, "CSockSession::tcpSend");
    assert(IsValid());
    if(outList_.empty())
        return true;
    CTcpConnSocket * conn = dynamic_cast<CTcpConnSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CTcpConnSocket");
        return false;
    }
    for(__BufList::iterator i = outList_.begin();i != outList_.end();){
        __Buffer & buf = *i;
        if(!buf.empty()){
            ssize_t sz = conn->SendData(buf);
            if(sz < 0){
                if(EAGAIN == errno || EWOULDBLOCK == errno){
                    ev_ |= EVENT_TCP_SEND;
                    break;
                }
                ERROR("SendData failed for buf="<<Tools::Dump(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
                return false;
            }else if(size_t(sz) < buf.size()){
                buf.erase(buf.begin(), buf.begin() + sz);
                ev_ |= EVENT_TCP_SEND;
                break;
            }
        }
        outList_.erase(i++);
    }
    return true;
}

bool CSockSession::udpSend()
{
    LOCAL_LOGGER(logger, "CSockSession::udpSend");
    assert(IsValid());
    if(outList_.empty())
        return true;
    CUdpSocket * conn = dynamic_cast<CUdpSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" into CUdpSocket");
        return false;
    }
    __BufList::iterator i = outList_.begin();
    __AddrList::iterator a = addrList_.begin();
    for(;i != outList_.end();){
        __Buffer & buf = *i;
        if(a == addrList_.end()){
            ERROR("no addr for buf="<<Tools::Dump(buf)<<" in sock="<<ToString());
            return false;
        }
        CSockAddr & addr = *a;
        if(!buf.empty()){
            ssize_t sz = conn->SendData(addr, buf);
            if(sz < 0){
                if(EAGAIN == errno || EWOULDBLOCK == errno){
                    ev_ |= EVENT_UDP_SEND;
                    break;
                }
                ERROR("SendData failed for buf="<<Tools::Dump(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
                return false;
            }
        }
        outList_.erase(i++);
        addrList_.erase(a++);
    }
    return true;
}

NS_SERVER_END

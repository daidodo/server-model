#include <sstream>

#include <Logger.h>
#include <DataStream.h>

#include "Command.h"
#include "SockSession.h"

NS_SERVER_BEGIN

//struct CSockSession

CSockSession::CSockSession(IFileDesc * fileDesc, const CRecvHelper & recvHelper)
    : fileDesc_(fileDesc)
    , recvHelper_(recvHelper)
    , needSz_(0)
    , ev_(0)
{}

CSockSession::~CSockSession()
{
    if(fileDesc_)
        IFileDesc::PutObject(fileDesc_);
}

std::string CSockSession::ToString() const
{
    std::ostringstream oss;
    oss<<"{fileDesc_="<<Tools::ToStringPtr(fileDesc_)
        <<", recvHelper_="<<recvHelper_.ToString()
        <<", ev_="<<Events::ToString(ev_)
        <<", outList_.size()="<<outList_.size()
        <<"}";
    return oss.str();
}

bool CSockSession::Accept(CSockSession *& client, __Events & events)
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
        addEvents(EVENT_ACCEPT);
        return true;
    }
    clientSock->SetLinger();
    clientSock->SetBlock(false);
    client = GetObject(&*clientSock, recvHelper_);
    if(!client){
        ERROR("no memory for sock session of clientSock="<<Tools::ToStringPtr(clientSock)<<", close it");
        return false;
    }
    clientSock.release();
    events = EVENT_IN;   //---------要根据process状态机决定
    return true;
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
    __Buffer buf;
    CSockAddr addr;
    while(getBuf(buf, addr)){
        assert(!buf.empty());
        ssize_t sz = file->Write(buf);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                addEvents(EVENT_WRITE);
                return putBuf(buf, addr, true);
            }
            ERROR("Write failed for buf="<<Tools::DumpHex(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else if(size_t(sz) < buf.size()){
            DEBUG("write half sz="<<sz<<" of buf="<<Tools::DumpHex(buf)<<" into file="<<Tools::ToStringPtr(file));
            buf.erase(buf.begin(), buf.begin() + sz);
            addEvents(EVENT_WRITE);
            return putBuf(buf, addr, true);
        }
        DEBUG("write buf="<<Tools::DumpHex(buf)<<" into file="<<Tools::ToStringPtr(file));
    }
    return true;
}

__Events CSockSession::Process(__CmdBase & cmd, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::Process");
    CCmdQuery & query = dynamic_cast<CCmdQuery &>(cmd);
    INFO("process query="<<query.ToString()<<", udpClientAddr="<<udpClientAddr.ToString()<<" from sock="<<ToString());
    CCmdResp resp(query);
    resp.Result();
    INFO("resp="<<resp.ToString()<<" for query="<<query.ToString()<<", udpClientAddr="<<udpClientAddr.ToString()<<" from sock="<<ToString());
    COutByteStream out;
    resp.Encode(out);
    __Buffer buf;
    out.ExportData(buf);
    AddOutBuf(buf, udpClientAddr);
    return EVENT_OUT;
}

bool CSockSession::RecvTcpCmd(__CmdBase *& cmd)
{
    LOCAL_LOGGER(logger, "CSockSession::RecvTcpCmd");
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
            ERROR("needSz_=0 is invalid for sock="<<ToString());
            return false;
        }
        ssize_t sz = conn->RecvData(recvBuf_, needSz_);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                addEvents(EVENT_TCP_RECV);
                break;
            }
            ERROR("RecvData(needSz_="<<needSz_<<") failed for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else if(sz == 0){
            WARN("client closed for sock="<<ToString());
            return false;
        }else if(size_t(sz) < needSz_){
            needSz_ -= sz;
            addEvents(EVENT_TCP_RECV);
            break;
        }else{
            const __OnDataArriveRet ret = recvHelper_.OnDataArrive(&recvBuf_[0], recvBuf_.size());
            switch(ret.first){
                case RR_COMPLETE:
                    DEBUG("recv buf="<<Tools::DumpHex(recvBuf_)<<" from conn="<<Tools::ToStringPtr(conn));
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

bool CSockSession::RecvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr)
{
    LOCAL_LOGGER(logger, "CSockSession::RecvUdpCmd");
    assert(IsValid());
    cmd = 0;
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
                addEvents(EVENT_UDP_RECV);
                break;
            }
            ERROR("RecvData(needSz_="<<needSz_<<") failed for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else{
            DEBUG("recv buf="<<Tools::DumpHex(recvBuf_)<<" from udpClientAddr="<<udpClientAddr.ToString()<<", conn="<<Tools::ToStringPtr(conn));
            const __OnDataArriveRet ret = recvHelper_.OnDataArrive(&recvBuf_[0], recvBuf_.size());
            if(ret.first == RR_ERROR){
                ERROR("onArrive() failed for sock="<<ToString());
                return false;
            }
            return decodeCmd(cmd, 0);
        }
    }
    return true;
}

bool CSockSession::decodeCmd(__CmdBase *& cmd, size_t left)
{
    LOCAL_LOGGER(logger, "CSockSession::decodeCmd");
    __DecodeCmd decoder = recvHelper_.DecodeCmd();
    assert(decoder && left < recvBuf_.size());
    size_t len = recvBuf_.size() - left;
    void * ret = decoder(&recvBuf_[0], len);
    if(!ret){
        ERROR("decode failed for recvBuf_="<<Tools::DumpHex(&recvBuf_[0], len)<<" from sock="<<ToString());
        return false;
    }
    cmd = reinterpret_cast<__CmdBase *>(ret);
    TRACE("decode cmd="<<Tools::ToStringPtr(cmd)<<" from buf="<<Tools::DumpHex(&recvBuf_[0], len)<<" from sock="<<ToString());
    if(left)
        recvBuf_.erase(recvBuf_.begin(), recvBuf_.begin() + len);
    else
        recvBuf_.clear();
    needSz_ = 0;
    return true;
}

bool CSockSession::SendTcpData()
{
    LOCAL_LOGGER(logger, "CSockSession::SendTcpData");
    assert(IsValid());
    CTcpConnSocket * conn = dynamic_cast<CTcpConnSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" to CTcpConnSocket in sock="<<ToString());
        return false;
    }
    __Buffer buf;
    CSockAddr addr;
    while(getBuf(buf, addr)){
        assert(!buf.empty());
        ssize_t sz = conn->SendData(buf);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                addEvents(EVENT_TCP_SEND);
                return putBuf(buf, addr, true);
            }
            ERROR("SendData failed for buf="<<Tools::DumpHex(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }else if(size_t(sz) < buf.size()){
            DEBUG("send half sz="<<sz<<" of buf="<<Tools::DumpHex(buf)<<" to conn="<<Tools::ToStringPtr(conn));
            buf.erase(buf.begin(), buf.begin() + sz);
            addEvents(EVENT_TCP_SEND);
            return putBuf(buf, addr, true);
        }
        DEBUG("send buf="<<Tools::DumpHex(buf)<<" to conn="<<Tools::ToStringPtr(conn));
    }
    return true;
}

bool CSockSession::SendUdpData()
{
    LOCAL_LOGGER(logger, "CSockSession::SendUdpData");
    assert(IsValid());
    CUdpSocket * conn = dynamic_cast<CUdpSocket *>(fileDesc_);
    if(!conn){
        ERROR("cannot cast fileDesc_="<<Tools::ToStringPtr(fileDesc_)<<" to CUdpSocket in sock="<<ToString());
        return false;
    }
    __Buffer buf;
    CSockAddr addr;
    while(getBuf(buf, addr)){
        assert(!buf.empty());
        if(addr.IsValid()){
            ERROR("invalid addr="<<addr.ToString()<<" for buf="<<Tools::DumpHex(buf));
            return false;
        }
        ssize_t sz = conn->SendData(addr, buf);
        if(sz < 0){
            if(EAGAIN == errno || EWOULDBLOCK == errno){
                addEvents(EVENT_UDP_SEND);
                return putBuf(buf, addr, true);
            }
            ERROR("SendData failed for buf="<<Tools::DumpHex(buf)<<" for sock="<<ToString()<<IFileDesc::ErrMsg());
            return false;
        }
        DEBUG("send buf="<<Tools::DumpHex(buf)<<" to addr="<<addr.ToString()<<", conn="<<Tools::ToStringPtr(conn));
    }
    return true;
}

bool CSockSession::getBuf(__Buffer & buf, CSockAddr & addr)
{
    buf.clear();
    addr.Reset();
    __Guard g(sendLock_);
    if(outList_.empty())
        return false;
    __BufList::iterator i = outList_.begin();
    __AddrList::iterator j = addrList_.begin();
    while(i != outList_.end()){
        buf.swap(*i);
        outList_.erase(i++);
        if(j != addrList_.end()){
            addr.Swap(*j);
            addrList_.erase(j++);
        }
        if(!buf.empty())
            break;
    }
    return true;
}

bool CSockSession::putBuf(__Buffer & buf, CSockAddr & addr, bool front)
{
    LOCAL_LOGGER(logger, "CSockSession::putBuf");
    assert(IsValid());
    DEBUG("add buf="<<Tools::DumpHex(buf)<<", addr="<<addr.ToString()<<" to send list, front="<<front);
    bool isUdp = (FD_UDP == fileDesc_->Type());
    if(isUdp && !addr.IsValid()){
        ERROR("invalid addr="<<addr.ToString()<<" for sock="<<ToString());
        return false;
    }
    __Guard g(sendLock_);
    __BufList::iterator i = outList_.insert((front ? outList_.begin() : outList_.end()), __Buffer());
    i->swap(buf);
    if(isUdp){
        __AddrList::iterator j = addrList_.insert((front ? addrList_.begin() : addrList_.end()), CSockAddr());
        j->Swap(addr);
    }
    return true;
}

NS_SERVER_END

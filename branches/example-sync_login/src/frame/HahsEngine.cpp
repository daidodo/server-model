#include <Logger.h>

#include "AsyncNotify.h"
#include "AsyncIO.h"
#include "CmdHandler.h"
#include "HahsEngine.h"

NS_SERVER_BEGIN

//struct CHahsEngine

CHahsEngine::CHahsEngine()
    : notify_(0)
    , io_(0)
    , handler_(0)
{}

bool CHahsEngine::AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper)
{
    typedef CSharedPtr<CListenSocket, false> __FileGuard;
    LOCAL_LOGGER(logger, "CHahsEngine::AddTcpListen");
    assert(bindAddr.IsValid());
    DEBUG("bindAddr="<<bindAddr.ToString()<<", recvHelper=@"<<&recvHelper);
    __FileGuard file = dynamic_cast<__FileGuard::pointer>(IFileDesc::GetObject(FD_TCP_LISTEN));
    if(!file){
        ERROR("cannot get file object for bindAddr="<<bindAddr.ToString());
        return false;
    }
    if(!file->Listen(bindAddr, false)){
        ERROR("cannot listen bindAddr="<<bindAddr.ToString()<<IFileDesc::ErrMsg());
        return false;
    }
    if(!file->SetBlock(false)){
        ERROR("set non-blocking failed for file="<<Tools::ToStringPtr(file));
        return false;
    }
    TRACE("file="<<Tools::ToStringPtr(file));
    __SockPtr sock = CSockSession::GetObject(&*file, recvHelper);
    if(!sock){
        ERROR("cannot get sock session for file="<<Tools::ToStringPtr(file));
        return false;
    }
    file.release();
    return addSock(sock, EVENT_IN);
}

bool CHahsEngine::AddTcpConn(const CSockAddr & connectAddr, const CRecvHelper & recvHelper)
{
    typedef CSharedPtr<CTcpConnSocket, false> __FileGuard;
    LOCAL_LOGGER(logger, "CHahsEngine::AddTcpConn");
    assert(connectAddr.IsValid());
    DEBUG("connectAddr="<<connectAddr.ToString()<<", recvHelper=@"<<&recvHelper);
    __FileGuard file = dynamic_cast<__FileGuard::pointer>(IFileDesc::GetObject(FD_TCP_CONN));
    if(!file){
        ERROR("cannot get file object for connectAddr="<<connectAddr.ToString());
        return false;
    }
    if(!file->Connect(connectAddr)){
        ERROR("cannot connect addr="<<connectAddr.ToString()<<IFileDesc::ErrMsg());
        return false;
    }
    file->SetLinger();
    if(!file->SetBlock(false)){
        ERROR("set non-blocking failed for file="<<Tools::ToStringPtr(file));
        return false;
    }
    DEBUG("file="<<Tools::ToStringPtr(file));
    __SockPtr sock = CSockSession::GetObject(&*file, recvHelper);
    if(!sock){
        ERROR("cannot get sock session for file="<<Tools::ToStringPtr(file));
        return false;
    }
    file.release();
    return addSock(sock, EVENT_IN);     //根据sock的处理步骤决定
}

bool CHahsEngine::AddUdpConn(const CSockAddr & bindAddr, const CSockAddr & connectAddr, const CRecvHelper & recvHelper)
{
    typedef CSharedPtr<CUdpSocket, false> __FileGuard;
    LOCAL_LOGGER(logger, "CHahsEngine::AddUdpConn");
    DEBUG("bindAddr="<<bindAddr.ToString()<<", connectAddr="<<connectAddr.ToString()<<", recvHelper=@"<<&recvHelper);
    __FileGuard file = dynamic_cast<__FileGuard::pointer>(IFileDesc::GetObject(FD_UDP));
    if(!file){
        ERROR("cannot get file object for connectAddr="<<connectAddr.ToString());
        return false;
    }
    if(bindAddr.IsValid()){
        if(!file->Bind(bindAddr)){
            ERROR("cannot bind addr="<<bindAddr.ToString()<<IFileDesc::ErrMsg());
            return false;
        }
    }
    if(connectAddr.IsValid()){
        if(!file->Connect(connectAddr)){
            ERROR("cannot connect addr="<<connectAddr.ToString()<<IFileDesc::ErrMsg());
            return false;
        }
    }
    if(!file->SetBlock(false)){
        ERROR("set non-blocking failed for file="<<Tools::ToStringPtr(file));
        return false;
    }
    DEBUG("file="<<Tools::ToStringPtr(file));
    __SockPtr sock = CSockSession::GetObject(&*file, recvHelper);
    if(!sock){
        ERROR("cannot get sock session for file="<<Tools::ToStringPtr(file));
        return false;
    }
    file.release();
    return addSock(sock, EVENT_IN);     //根据sock的处理步骤决定
}

bool CHahsEngine::Run(const CHahsEnginParams & params)
{
    LOCAL_LOGGER(logger, "CHahsEngine::Run");
    //check
    if(notify_ || io_ || handler_){
        ERROR("cannot run again");
        return false;
    }
    //allocate
    notify_ = new CAsyncNotify(params.notifyStackSz_, *this);
    io_ = new CAsyncIO(params.ioStackSz_, *this);
    if(!notify_ || !io_){
        FATAL("cannot get CAsyncNotify/CAsyncIO objects");
        uninit();
        return false;
    }
    if(params.needHandler_){
        handler_ = new CCmdHandler(params.handlerStackSz_, *this);
        if(!handler_){
            FATAL("cannot get CCmdHandler objects");
            uninit();
            return false;
        }
    }
    //init
    if(!notify_->Init(params.maxFdNum_, params.epollTimeoutMs_)){
        FATAL("notify Init(maxFdNum_="<<params.maxFdNum_<<", epollTimeoutMs_="<<params.epollTimeoutMs_<<") failed");
        uninit();
        return false;
    }
    if(!io_->Init()){
        FATAL("io Init() failed");
        uninit();
        return false;
    }
    if(handler_ && !handler_->Init(params.handlerThreadMax_)){
        FATAL("handler Init(handlerThreadMax_="<<params.handlerThreadMax_<<") failed");
        uninit();
        return false;
    }
    //start
    if(handler_ && 0 > handler_->StartThreads("CmdHandler")){
        FATAL("start handler threads failed");
        exit(1);
    }
    if(0 > notify_->StartThreads("AsyncNotify")){
        FATAL("start notify thread failed");
        exit(1);
    }
    if(0 > io_->StartThreads("AsyncIO")){
        FATAL("start io thread failed");
        exit(1);
    }
    WARN("service start");
    return true;
}

void CHahsEngine::WaitAll()
{
    assert(notify_ && io_);
    notify_->WaitAll();
    io_->WaitAll();
    if(handler_)
        handler_->WaitAll();
}

void CHahsEngine::uninit()
{
    delete notify_;
    delete io_;
    notify_ = 0;
    io_ = 0;
    if(handler_){
        delete handler_;
        handler_ = 0;
    }
}

bool CHahsEngine::addSock(__SockPtr & sock, __Events ev)
{
    LOCAL_LOGGER(logger, "CHahsEngine::addSock");
    assert(sock);
    if(!sock->IsValid()){
        ERROR("invalid sock="<<Tools::ToStringPtr(sock)<<", ev="<<Events::ToString(ev));
        return false;
    }
    INFO("add sock="<<Tools::ToStringPtr(sock)<<", ev="<<Events::ToString(ev)<<" to engine");
    const int fd = sock->Fd();
    assert(fd >= 0);
    fdSockMap_.SetSock(fd, sock);
    if(ev){
        if(!eventQue_.Push(CFdEvent(fd, ev), 500)){
            ERROR("eventQue_.Push(fd="<<fd<<", ev="<<Events::ToString(ev)<<") failed for sock="<<Tools::ToStringPtr(sock));
            return false;
        }
    }
    return true;
}

NS_SERVER_END

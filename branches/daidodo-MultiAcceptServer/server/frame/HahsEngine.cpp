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

CHahsEngine::~CHahsEngine()
{
    uninit();
}

bool CHahsEngine::AddTcpListen(const CSockAddr & bindAddr, const CRecvHelper & recvHelper)
{
    typedef CSharedPtr<CListenSocket, false> __TcpListenPtr;
    LOCAL_LOGGER(logger, "CHahsEngin::AddTcpListen");
    INFO("bindAddr="<<bindAddr.ToString()<<", recvHelper="<<recvHelper.ToString());
    __TcpListenPtr listen = dynamic_cast<CListenSocket *>(IFileDesc::GetObject(FD_TCP_LISTEN));
    if(!listen){
        ERROR("cannot get CListenSocket object for bindAddr="<<bindAddr.ToString());
        return false;
    }
    if(!listen->Listen(bindAddr, false)){
        ERROR("cannot listen bindAddr="<<bindAddr.ToString()<<IFileDesc::ErrMsg());
        return false;
    }
    INFO("listen="<<Tools::ToStringPtr(listen));
    __SockPtr sock = CSockSession::GetObject(&*listen, recvHelper);
    if(!sock){
        ERROR("cannot get sock session for listen="<<Tools::ToStringPtr(listen));
        return false;
    }
    sock->Events(EVENT_ACCEPT);
    const int fd = sock->Fd();
    INFO("fd="<<fd<<", sock="<<Tools::ToStringPtr(sock));
    assert(fd >= 0);
    fdSockMap_.SetSock(fd, sock);
    if(!addingQue_.Push(fd, 500)){
        ERROR("addingQue_.Push(fd="<<fd<<") failed for sock="<<Tools::ToStringPtr(sock));
        return false;
    }
    return true;
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
    handler_ = new CCmdHandler(params.handlerStackSz_, *this);
    if(!notify_ || !io_ || !handler_){
        FATAL("cannot get CAsyncNotify/CAsyncIO/CCmdHandler objects");
        uninit();
        return false;
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
    if(!handler_->Init(params.handlerThreadMax_)){
        FATAL("handler Init(handlerThreadMax_="<<params.handlerThreadMax_<<") failed");
        uninit();
        return false;
    }
    //start
    if(0 > handler_->StartThreads("CmdHandler")){
        FATAL("start handler threads failed");
        exit(1);
    }
    if(0 > io_->StartThreads("AsyncIO")){
        FATAL("start io thread failed");
        exit(1);
    }
    if(0 > notify_->StartThreads("AsyncNotify")){
        FATAL("start notify thread failed");
        exit(1);
    }
    return false;
}

void CHahsEngine::WaitAll()
{
    assert(notify_ && io_ && handler_);
    notify_->WaitAll();
    io_->WaitAll();
    handler_->WaitAll();
}

void CHahsEngine::uninit()
{
    delete notify_;
    delete io_;
    delete handler_;
    notify_ = 0;
    io_ = 0;
    handler_ = 0;
}

NS_SERVER_END

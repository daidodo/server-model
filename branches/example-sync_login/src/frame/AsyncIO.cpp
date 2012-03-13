#include <Logger.h>
#include <IterAdapter.h>

#include "CmdSession.h"
#include "AsyncIO.h"

NS_SERVER_BEGIN

CAsyncIO::CAsyncIO(size_t stackSz, CHahsEngine & engine)
    : CThreadPool(1, stackSz)
    , addingQue_(engine.addingQue_)
    , eventQue_(engine.eventQue_)
    , fdSockMap_(engine.fdSockMap_)
    , queryCmdQue_(engine.queryCmdQue_)
{}

int CAsyncIO::doIt()
{
    LOCAL_LOGGER(logger, "CAsyncIO::doIt");
    CListParams listParams;
    __SockPtrList & sockList = listParams.sockList_;
    __FdEventList & eventList = listParams.eventList_;
    __FdList & addingList = listParams.addingList_;
    __QueryCmdList & queryCmdList = listParams.queryCmdList_;
    for(;;){
        //pop all events
        if(!eventQue_.PopAll(eventList)){
            WARN("eventQue_.PopAll() failed");
            continue;
        }
        TRACE("eventQue_.PopAll() size="<<eventList.size());
        if(eventList.empty())
            continue;
        //get sockets
        sockList.resize(eventList.size());
        fdSockMap_.GetSock(const_iter_adapt_fun<int>(eventList.begin(), CFdEvent::ExtractFd)
                , const_iter_adapt_fun<int>(eventList.end(), CFdEvent::ExtractFd)
                , sockList.begin());
        __FdEventList::const_iterator i = eventList.begin();
        __SockPtrList::iterator sock_i = sockList.begin();
        for(;i != eventList.end();++i, ++sock_i){
            const int fd = i->Fd();
            __SockPtr & sock = *sock_i;
            //validate fd and socket
            if(!sock || !sock->IsValid()){
                ERROR("fd="<<fd<<" is not sock="<<Tools::ToStringPtr(sock)<<" before handle events, ignore it");
                continue;
            }
            //handle events
            __Events oldEv = sock->Events();
            TRACE("handle ev="<<Events::ToString(i->Events())<<" from sock="<<Tools::ToStringPtr(sock));
            bool add = false;
            if(Events::NeedClose(i->Events()))
                add = true;
            else{
                bool ok = true;
                if(ok && Events::CanOutput(i->Events()))
                    ok = handleOutput(sock);
                if(ok && Events::CanInput(i->Events()))
                    ok = handleInput(sock, listParams);
                if(!ok)
                    sock->Events(EVENT_CLOSE);
                if(oldEv != sock->Events())
                    add = true;
            }
            //update events
            if(add){
                TRACE("add fd="<<fd<<", ev="<<Events::ToString(sock->Events())<<" into addingList, oldEv="<<oldEv<<", sock="<<Tools::ToStringPtr(sock));
                addingList.push_back(fd);
            }
        }
        //flush addingList
        if(!addingList.empty()){
            TRACE("addingQue_.PushAll(size="<<addingList.size()<<")");
            if(!addingQue_.PushAll(addingList, 500)){
                ERROR("addingQue_.PushAll(size="<<addingList.size()<<") failed, close all sockets");
                fdSockMap_.CloseSock(addingList.begin(), addingList.end());
            }
        }
        //flush queryCmdList
        if(!queryCmdList.empty()){
            TRACE("queryCmdQue_.PushAll(size="<<queryCmdList.size()<<")");
            if(!queryCmdQue_.PushAll(queryCmdList, 200)){
                ERROR("queryCmdQue_.PushAll(size="<<queryCmdList.size()<<") failed");
            }
        }
    }
    return 0;
}

bool CAsyncIO::handleOutput(__SockPtr & sock)
{
    assert(sock);
    switch(sock->FileType()){
        case FD_FILE:return sock->WriteData();
        case FD_TCP_CONN:return sock->SendTcpData();
        case FD_UDP:return sock->SendUdpData();
        default:;
    }
    LOCAL_LOGGER(logger, "CAsyncIO::handleOutput");
    ERROR("cannot output for sock="<<Tools::ToStringPtr(sock));
    return false;
}

bool CAsyncIO::handleInput(__SockPtr & sock, CListParams & listParams)
{
    assert(sock);
    switch(sock->FileType()){
        case FD_FILE:assert(0);break;   //-------not implemented
        case FD_TCP_LISTEN:return handleAccept(sock, listParams);
        case FD_TCP_CONN:return handleRecv(sock, listParams, false);
        case FD_UDP:return handleRecv(sock, listParams, true);
        default:;
    }
    LOCAL_LOGGER(logger, "CAsyncIO::handleInput");
    ERROR("cannot input for sock="<<Tools::ToStringPtr(sock));
    return false;
}

bool CAsyncIO::handleRecv(__SockPtr & sock, CListParams & listParams, bool isUdp)
{
    assert(sock);
    CSockAddr udpClientAddr;
    for(CAnyPtr cmd;;){
        if(!(isUdp ?
                sock->RecvUdpCmd(cmd, udpClientAddr) :
                sock->RecvTcpCmd(cmd)))
            return false;
        if(!cmd)
            break;
        if(!handleCmd(sock, cmd, udpClientAddr, listParams))
            return false;
    }
    return true;
}

bool CAsyncIO::handleAccept(__SockPtr & sock, CListParams & listParams)
{
    typedef CHahsEngine::__SockSession __SockSession;
    LOCAL_LOGGER(logger, "CAsyncIO::handleAccept");
    assert(sock);
    for(;;){
        __SockSession * client = 0;
        __Events ev = 0;
        if(!sock->Accept(client, ev)){
            ERROR("accept error for sock="<<Tools::ToStringPtr(sock));
            return false;
        }
        if(!client)
            break;
        __SockPtr ptr(client);
        INFO("new client="<<Tools::ToStringPtr(client)<<" arrived, ev="<<Events::ToString(ev));
        if(!ev){    //no events, only warning
            WARN("inavlid ev="<<Events::ToString(ev)<<" for accepted client="<<Tools::ToStringPtr(client));
        }
        const int fd = client->Fd();
        fdSockMap_.SetSock(fd, ptr);
        TRACE("add fd="<<fd<<", ev="<<Events::ToString(ev)<<" into eventList for client="<<Tools::ToStringPtr(client));
        listParams.eventList_.push_back(CFdEvent(fd, ev));
        listParams.sockList_.push_back(ptr);
    }
    return true;
}

bool CAsyncIO::handleCmd(__SockPtr & sock, const CAnyPtr & cmd, CSockAddr & udpClientAddr, CListParams & listParams)
{
    typedef CHahsEngine::__CmdSession __CmdSession;
    typedef CSharedPtr<__CmdSession, false> __CmdSessionPtr;
    LOCAL_LOGGER(logger, "CAsyncIO::handleCmd");
    assert(sock && cmd);
    const int fd = sock->Fd();
    U32 fingerPrint = sock->FingerPrint();
    if(!sock->IsValid()){
        ERROR("sock="<<Tools::ToStringPtr(sock)<<" is invalid before push cmd="<<cmd.ToString());
        return false;
    }
    __CmdSessionPtr session(__CmdSession::GetObject(fd, fingerPrint, cmd, sock->RecvHelper(), udpClientAddr));    //guard
    TRACE("push cmd session="<<Tools::ToStringPtr(session)<<" into queryCmdList");
    listParams.queryCmdList_.push_back(&*session);
    session.release();
    return true;
}

NS_SERVER_END

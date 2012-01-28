#include <Logger.h>
#include <IterAdapter.h>

#include "HahsEngine.h"
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
    __SockPtrList sockList;
    __FdEventList eventList;
    __FdList addingList;
    for(;;){
        //pop all events
        if(!eventQue_.PopAll(eventList)){
            WARN("eventQue_.PopAll() failed");
            continue;
        }
        TRACE("get eventList.size()="<<eventList.size());
        if(eventList.empty())
            continue;
        //get sockets
        sockList.resize(eventList.size());
        fdSockMap_.GetSock(const_iter_adapt_fun<int>(eventList.begin(), __FdEvent::ExtractFd)
                , const_iter_adapt_fun<int>(eventList.end(), __FdEvent::ExtractFd)
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
            DEBUG("handle ev="<<Events::ToString(i->Events())<<" from sock="<<Tools::ToStringPtr(sock));
            bool ok = true;
            if(ok && Events::CanOutput(i->Events()))
                ok = handleOutput(sock);
            if(ok && Events::CanInput(i->Events()))
                ok = handleInput(sock, addingList);
            if(!ok)
                sock->Events(EVENT_CLOSE);
            //update events
            if(oldEv != sock->Events()){
                DEBUG("add fd="<<fd<<", ev="<<Events::ToString(sock->Events())<<" into addingList, oldEv="<<oldEv);
                addingList.push_back(fd);
            }
        }
        //flush addingList
        if(!addingList.empty()){
            TRACE("addingQue_.PushAll(size="<<addingList.size()<<")");
            if(!addingQue_.PushAll(addingList, 500)){
                ERROR("addingQue_.PushAll() failed, close all sockets");
                fdSockMap_.CloseSock(addingList.begin(), addingList.end());
            }
        }
    }
    return 0;
}

bool CAsyncIO::handleOutput(__SockPtr & sock)
{
    assert(sock);
    if(Events::CanSend(sock->Events()))
        return sock->SendBuffer();
    else if(Events::CanWrite(sock->Events()))
        return sock->WriteData();
    return true;
}

bool CAsyncIO::handleInput(__SockPtr & sock, __FdList & addingList)
{
    assert(sock);
    if(Events::CanAccept(sock->Events())){
        return handleAccept(sock, addingList);
    }else if(Events::CanRecv(sock->Events())){
        CSockAddr udpClientAddr;
        for(__CmdBase * cmd = 0;;){
            if(!sock->RecvCmd(cmd, udpClientAddr))
                return false;
            if(!cmd)
                break;
            if(!handleCmd(sock, cmd, udpClientAddr))
                return false;
        }
    }else if(Events::CanRead(sock->Events())){
        assert(0);
    }
    return true;
}

bool CAsyncIO::handleAccept(__SockPtr & sock, __FdList & addingList)
{
    LOCAL_LOGGER(logger, "CAsyncIO::handleAccept");
    assert(sock);
    for(__SockSession * client;;){
        if(!sock->Accept(client)){
            ERROR("accept error for sock="<<Tools::ToStringPtr(sock));
            return false;
        }
        if(!client)
            break;
        const int fd = client->Fd();
        INFO("new client="<<Tools::ToStringPtr(client)<<" arrived");
        __SockPtr ptr(client);
        fdSockMap_.SetSock(fd, ptr);
        TRACE("add fd="<<fd<<", client="<<Tools::ToStringPtr(client)<<" into addingList");
        addingList.push_back(fd);
    }
    return true;
}

bool CAsyncIO::handleCmd(__SockPtr & sock, __CmdBase * cmd, CSockAddr & udpClientAddr)
{
    typedef CSharedPtr<__CmdSession, false> __CmdSessionPtr;
    LOCAL_LOGGER(logger, "CAsyncIO::handleCmd");
    assert(sock && cmd);
    __CmdSessionPtr session(__CmdSession::GetObject(sock, cmd, udpClientAddr));    //guard
    TRACE("push cmd session="<<Tools::ToStringPtr(session)<<" into queryCmdQue_");
    if(!queryCmdQue_.Push(&*session, 200)){
        WARN("queryCmdQue_.Push(session="<<Tools::ToStringPtr(session)<<") failed, destroy it");
    }else
        session.release();
    return true;
}

NS_SERVER_END

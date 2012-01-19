#include <Logger.h>
#include <IterAdapter.h>

#include "AsyncIO.h"

NS_SERVER_BEGIN

CAsyncIO::CAsyncIO(const CAsyncIoCtorParams & params)
    : CThreadPool(1, params.stackSize_)
    , addingQue_(params.addingQue_)
    , eventQue_(params.eventQue_)
    , fdSockMap_(params.fdSockMap_)
    , queryCmdQue_(params.queryCmdQue_)
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
            if(!sock || sock->Fd() != fd){
                ERROR("fd="<<fd<<" is not sock="<<Tools::ToStringPtr(sock)<<" handle events, ignore it");
                continue;
            }
            //handle events
            __Events oldEv = sock->Events();
            bool ok = true;
            if(ok && Events::CanOutput(i->Events()))
                ok = handleOutput(sock);
            if(ok && Events::CanInput(i->Events()))
                ok = handleInput(sock, addingList);
            //update events
            if(oldEv != sock->Events())
                addingList.push_back(fd);
        }
        //flush addingList
        if(!addingQue_.PushAll(addingList, 500)){
            ERROR("addingQue_.PushAll() failed, close all sockets");
            fdSockMap_.CloseSock(addingList.begin(), addingList.end());
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
        for(;;){
            __CmdBase * cmd = 0;
            if(!sock->RecvCmd(cmd))
                return false;
            if(!cmd)
                break;
            if(!handleCmd(sock, cmd))
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
    __SockPtr client(sock->Accept());
    if(client){
        DEBUG("new client arrived "<<Tools::ToStringPtr(client));
        const int fd = client->Fd();
        DEBUG("set fdSockMap_["<<fd<<"]="<<Tools::ToStringPtr(client)<<" and push into addingList");
        fdSockMap_.SetSock(fd, client);
        client->Events(EVENT_RECV);
        addingList.push_back(fd);
    }
    return true;
}

bool CAsyncIO::handleCmd(__SockPtr & sock, __CmdBase * cmd)
{
    typedef CSharedPtr<__CmdBase, false> __CmdBasePtr;
    LOCAL_LOGGER(logger, "CAsyncIO::handleCmd");
    assert(sock && cmd);
    __CmdBasePtr g(cmd);    //guard
    if(!queryCmdQue_.Push(__CmdTriple(cmd, sock->Fd(), sock), 200)){
        WARN("queryCmdQue_.Push(cmd="<<Tools::ToStringPtr(cmd)<<") from sock="<<Tools::ToStringPtr(sock)<<" failed, destroy cmd");
    }else
        g.release();
    return true;
}

NS_SERVER_END

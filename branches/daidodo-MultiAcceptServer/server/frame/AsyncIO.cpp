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
    __FdEventList eventList, addingList;
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
            if(i->Writable()){
                if(!handleSend(sock, addingList))
                    break;
            }
            if(i->Readable()){
                if(!handleRecv(sock, addingList))
                    break;
            }
        }
    }
    return 0;
}

bool CAsyncIO::handleSend(__SockPtr & sock, __FdEventList & addingList)
{
    assert(sock);
    const int fd = sock->Fd();
    int ret = sock->SendBuffer();
    switch(ret){
        case __CmdSock::RET_IO_ERROR:
            addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_CLOSE));
            return false;
        case __CmdSock::RET_IO_RETRY:
            if(!sock->WriteEvent())
                addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_WRITE_ADD));
            break;
        case __CmdSock::RET_COMPLETE:
            break;
        default:assert(0);
    }
    return true;
}

bool CAsyncIO::handleRecv(__SockPtr & sock, __FdEventList & addingList)
{
    assert(sock);
    if(sock->Acceptable())
        return handleAccept(sock, addingList);
    const int fd = sock->Fd();
    CCmdBase * cmd;
    for(bool loop = true;loop;){
        int ret = sock->RecvCmd(cmd);
        switch(ret){
            case __CmdSock::RET_IO_ERROR:
                addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_CLOSE));
                return false;
            case __CmdSock::RET_IO_RETRY:
                if(!sock->ReadEvent())
                    addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_READ_ADD));
                loop = false;
                break;
            case __CmdSock::RET_COMPLETE:
                if(!handleCmd(sock, cmd)){
                    addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_CLOSE));
                    return false;
                }
                break;
            default:assert(0);
        }
    }
    return true;
}

bool CAsyncIO::handleAccept(__SockPtr & sock, __FdEventList & addingList)
{
    LOCAL_LOGGER(logger, "CAsyncIO::handleAccept");
    assert(sock);
    __SockPtr client(sock->Accept());
    if(client){
        DEBUG("new client arrived "<<Tools::ToStringPtr(client));
        client->Socket().SetLinger();
        client->Socket().SetBlock(false);
        const int fd = client->Fd();
        DEBUG("set fdSockMap_["<<fd<<"]="<<Tools::ToStringPtr(client)<<" and push into addingList");
        fdSockMap_.SetSock(fd, client);
        addingList.push_back(__FdEvent(fd, __FdEvent::EVENT_READ));
    }
    return true;
}

bool CAsyncIO::handleCmd(__SockPtr & sock, CCmdBase * cmd)
{
    typedef CSharedPtr<CCmdBase, false> __CmdBasePtr;
    LOCAL_LOGGER(logger, "CAsyncIO::handleCmd");
    assert(sock && cmd);
    __CmdBasePtr g(cmd);    //guard only
    if(!queryCmdQue_.Push(__CmdTriple(cmd, sock->Fd(), sock), 200)){
        WARN("queryCmdQue_.Push(cmd="<<Tools::ToStringPtr(cmd)<<") from sock="<<Tools::ToStringPtr(sock)<<" failed, destroy cmd");
    }else
        g.release();
    return true;
}

NS_SERVER_END

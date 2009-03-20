#include <common/Logger.h>
#include <common/Tools.h>
#include "TcpServer.h"

NS_SERVER_BEGIN

CTcpServer::CTcpServer(CMainServer & mainServer)
    : CThreadPool(mainServer.tcpServerThreadCount_,mainServer.tcpServerStackSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , removeFdQue_(mainServer.removeFdQue_)
    , eventFdQue_(mainServer.eventFdQue_)
    , EVENT_QUE_SZ_(mainServer.tcpServerThreadCount_)
    , queryCmdQue_(mainServer.queryCmdQue_)
    , useEpoll_(mainServer.useEpoll_)
    , index_(0)
    , stats_(mainServer.serverStatsOn_ ? new __Stats : 0)
{}

CTcpServer::~CTcpServer()
{
    if(stats_)
        delete stats_;
}

int CTcpServer::doIt()
{
    typedef __FdEventQue::container_type    __FdEventList;
    typedef __FdEventList::const_iterator   __Iter;
    typedef __FdQue::container_type         __FdList;
    LOCAL_LOGGER(logger,"CTcpServer::doIt");
    __FdEventQue & eventFdQue = getEventQue();
    __FdEventList addingQue;
    __FdList removeQue;
    for(__FdEventList cont;;){
        if(!eventFdQue.PopAll(cont)){
            WARN("pop all from eventFdQue failed");
			continue;
        }
        __Active active(Cnt());	//活跃线程计数
        U32 cc = 0,rc = 0,hc = 0,bc = 0,sc = 0,se = 0,su = 0;   //统计
        for(__Iter i = cont.begin();i != cont.end();++i){
            const int & fd = i->fd_;
            DEBUG("tcp request from fd="<<fd<<", revent_="<<i->event_);
            __SockPtr pSock;
            fdSockMap_.GetSock(fd,pSock);
            if(!pSock || pSock->FD() != fd){
                WARN("fdSockMap[fd="<<fd<<"]="<<Tools::ToStringPtr(pSock)
                    <<" is invalid");
                continue;
            }
            int br = 0; //0:none; 1:read; 2:error
            bool bw = true;
            if(i->Writable())
                bw = sendCmdData(*pSock,sc,se,su);
            if(i->Readable())
                br = (recvCmdData(fd,pSock,cc,rc,hc,bc) ? 1 : 2);
            if(br == 2) //error
                removeQue.push_back(fd);
            else if(!useEpoll_){    //poll needs re-adding fd and event
                int ev = 0;
                if(br == 1)
                    ev = __FdEvent::EVENT_READ;
                if(!bw)
                    ev |= __FdEvent::EVENT_WRITE;
                if(ev){
                    DEBUG("push fd="<<fd<<", events="<<ev<<" into addingQue");
                    addingQue.push_back(__FdEvent(fd,ev));
                }
            }
        }
        if(!useEpoll_ && !addingFdQue_.PushAll(addingQue,500)){
            ERROR("push all to addingFdQue_ failed, close all sockets");
            fdSockMap_.CloseSock(const_iter_adapt_fun<int>(addingQue.begin(),__FdEvent::ExtractFd)
                ,const_iter_adapt_fun<int>(addingQue.end(),__FdEvent::ExtractFd));
        }
        if(!removeFdQue_.PushAll(removeQue,500)){
            ERROR("push all to removeFdQue_ failed, close all sockets");
            fdSockMap_.CloseSock(removeQue.begin(),removeQue.end());
        }
        if(stats_)
            stats_->Put(cc,rc,hc,bc,sc,se,su);
    }
    FATAL_COUT("thread quit");
    return 0;
}

CTcpServer::__FdEventQue & CTcpServer::getEventQue()
{
    LOCAL_LOGGER(logger,"CTcpServer::getEventQue");
    ASSERT(eventFdQue_,"eventFdQue_ is null");
    int i = index_++;
    ASSERT(size_t(i) < CThreadPool::ThreadCount(),"eventFdQue_ index="<<i
        <<" >= tcpServerThreadCount_="<<CThreadPool::ThreadCount());
    return eventFdQue_[i];
}

bool CTcpServer::sendCmdData(__CmdSock & sock,U32 & sc,U32 & se,U32 & su) const
{
    LOCAL_LOGGER(logger,"CTcpServer::sendCmdData");
    for(__Buf data;sock.GetSendData(data,useEpoll_);){
        if(data.empty())
            continue;
        ssize_t n = sock.SendData(data);
        if(size_t(n) != data.size()){
            if(n < 0){
                ERROR("send data to sock="<<sock.ToString()
                    <<" error"<<CSocket::ErrMsg());
                ++se;
            }else{
                DEBUG("send data part="<<Tools::DumpHex(&data[0],n)
                    <<" to sock="<<sock.ToString()<<" succ");
                data.erase(data.begin(),data.begin() + n);
                ++su;
            }
            sock.PutSendData(data,false);
            return false;
        }
        DEBUG("send data="<<Tools::DumpHex(data)<<" to sock="
            <<sock.ToString()<<" succ");
        ++sc;
    }
    return true;
}

bool CTcpServer::recvCmdData(int fd,__SockPtr & pSock,U32 & cc,U32 & rc,U32 & hc,U32 & bc)
{
    typedef __DZ_VECTOR(CCmdBuf) __CmdData;
    LOCAL_LOGGER(logger,"CTcpServer::recvCmdData");
    ASSERT(pSock,"pSock is null");
    __CmdData cmdData;
    int ret = __CmdSock::RET_CONTINUE;
    for(;ret == __CmdSock::RET_CONTINUE;){
        ret = pSock->RecvCommand();
        if(ret == __CmdSock::RET_COMPLETE){
            cmdData.push_back(CCmdBuf());
            pSock->ExportCmdData(cmdData.back());
            ret = __CmdSock::RET_CONTINUE;
        }
    }
    bool r = true;
    if(ret == __CmdSock::RET_CLOSED){           //peer closed
        DEBUG("pSock="<<Tools::ToStringPtr(pSock)<<" closed");
        pSock->ResetRecv();
        ++cc;
        r = false;
    }else if(ret == __CmdSock::RET_ERROR){      //recv error
        ERROR("recv from pSock="<<Tools::ToStringPtr(pSock)<<" error"
            <<CSocket::ErrMsg());
        pSock->ResetRecv();
        ++rc;
        r = false;
    }else if(ret == __CmdSock::RET_CMD_ERROR){  //cmd head error
        CCmdBuf cmdbuf;
        pSock->ExportCmdData(cmdbuf);
        ERROR("error cmd head buf="<<Tools::DumpVal(cmdbuf.Buf)<<" from pSock="
            <<Tools::ToStringPtr(pSock)<<", close it");
        ++hc;
        r = false;
    }//decode
    DEBUG("recv "<<cmdData.size()<<" cmd bufs from pSock="
        <<Tools::ToStringPtr(pSock));
    for(__CmdData::const_iterator i = cmdData.begin();i != cmdData.end();++i){
        QCmdBase * pCmd = QCmdBase::CreateCommand(i->Buf);
        if(!pCmd){                          //decode body error
            ERROR("decode command error from "<<Tools::ToStringPtr(pSock)
                <<", data buffer="<<Tools::DumpVal(i->Buf));
            ++bc;
            continue;
        }
        pCmd->UseHttp(i->UseHttp);
        DEBUG("push command="<<Tools::ToStringPtr<QCmdBase *>(pCmd)<<" from "
            <<Tools::ToStringPtr(pSock)<<" into queryCmdQue_");
        if(!queryCmdQue_.Push(__CmdTriple(pCmd,fd,pSock),200)){
            WARN("push into queryCmdQue_ failed for pCmd="<<Tools::ToStringPtr(pCmd)
                <<" from "<<Tools::ToStringPtr(pSock)<<", release pCmd");
            QCmdBase::ReleaseCommand(pCmd);
        }
    }
    return r;
}

NS_SERVER_END

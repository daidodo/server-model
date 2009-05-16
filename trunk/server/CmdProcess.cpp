#include <common/Logger.h>
#include <server/frame/CmdSock.h>
#include "CmdHandler.h"

NS_SERVER_BEGIN

void CCmdHandler::process(const __Job & cmdTriple)
{   //MAKE SURE THERE IS ONLY ONE WAY OUT
    LOCAL_LOGGER(logger,"CCmdHandler::process");
    QCmdBase * pCmd = cmdTriple.first;
    const int & fd = cmdTriple.second;
    __SockPtr pSock;
    fdSockMap_.GetSock(fd,pSock);
    if(cmdTriple.third && (!pSock || pSock != cmdTriple.third)){
        WARN("before process (command=@"<<pCmd<<",fd="<<fd<<"), old socket="
            <<Tools::ToStringPtr(cmdTriple.third)<<" is replaced by new socket="
            <<Tools::ToStringPtr(pSock)<<", abandon it");
    }else{
        INFO("process command("<<ICommand::CommandName(pCmd->CmdType())<<") from "
            <<Tools::ToStringPtr(pSock));
        __Buf respdata;
        CCommandStats stats(pCmd->CmdType(),true);
        switch(pCmd->CmdType()){    //ADD NEW COMMAND HERE
            case CMD_QUERY:{
                processCmd(dynamic_cast<CQueryCmd &>(*pCmd),respdata,stats);
                break;}
            default:
                ERROR("unknown command type="<<pCmd->CmdType()<<" from "
                    <<Tools::ToStringPtr(pSock)<<" cmd="<<Tools::ToStringPtr(pCmd));
        }
        if(cmdTriple.third && !respdata.empty()){
            fdSockMap_.GetSock(fd,pSock);   //在processCmd时,fd的pSock可能被替换了
            if(!pSock || pSock != cmdTriple.third){
                ERROR("after process (command=@"<<pCmd<<Tools::ToStringPtr(pCmd)<<",fd="
                    <<fd<<") old socket="<<Tools::ToStringPtr(cmdTriple.third)
                    <<" is replaced by new socket="<<Tools::ToStringPtr(pSock)<<", ignore it");
                stats.Succ(false);
            }else if(pSock->PutSendData(respdata)){
                if(useEpoll_){  //push to eventFdQue_
                    int i = fd % EVENT_QUE_SZ_;
                    int ev = __FdEvent::EVENT_WRITE;
                    DEBUG("push fd="<<fd<<", event="<<ev<<" into eventFdQue_["<<i<<"]");
                    if(!eventFdQue_[i].Push(__FdEvent(fd,ev),500)){
                        ERROR("push fd="<<fd<<", event=EVENT_WRITE into eventFdQue_["
                            <<i<<"] failed");
                    }
                }
            }else{  //send flag not set, push to addingFdQue_
                int ev = __FdEvent::EVENT_WRITE;
                DEBUG("push fd="<<fd<<", event="<<ev<<" into addingFdQue_");
                if(!addingFdQue_.Push(__FdEvent(fd,ev),1000)){
                    ERROR("push fd="<<fd<<", event=EVENT_WRITE into addingFdQue_ failed");
                }
            }
        }
    }
    QCmdBase::ReleaseCommand(pCmd);
}

void CCmdHandler::processCmd(CQueryCmd & cmd,__Buf & respdata,CCommandStats & stats)
{
    LOCAL_LOGGER(logger,"CCmdHandler::processCmd(CQueryCmd &)");
    INFO("query command="<<cmd.ToString());
    CQueryRespCmd resp(cmd);
    resp.result_ = 0;
    resp.fileHash_ = cmd.fileHash_;
    resp.clientHash_ = cmd.clientHash_;
    resp.peerId_.push_back(cmd.peerId_);
    resp.peerId_.push_back(cmd.peerId_ + "abc");
    resp.peerId_.push_back(cmd.peerId_ + "efg");
    INFO("resp cmd="<<resp.ToString());
    readyForSend(resp,respdata);
}

NS_SERVER_END

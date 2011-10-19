#include <errno.h>
#include <sys/socket.h>
#include <common/Logger.h>
#include <common/DataStream.h>
#include "CmdSock.h"

NS_SERVER_BEGIN

CCmdSock * CCmdSock::GetObject()
{
    CCmdSock * ret = allocator_type().allocate(1);
    return new (ret) CCmdSock;
}

void CCmdSock::PutObject(CCmdSock *& p)
{
    Tools::Destroy(p,allocator_type());
}

CCmdSock::CCmdSock()
    : recv_left_(QCmdBase::HEAD_LEN)
    , recvStatus_(STATUS_START)
    , setSendFlag_(false)
{}

int CCmdSock::RecvCommand(bool block,int cmdtype)
{
    size_t oldsz = recv_data_.size();
    if(!recv_left_)
        return RET_COMPLETE;
    ssize_t n =  RecvData(recv_data_,recv_left_,block);
    if(n > 0){      //recv succ
        if(U32(n) < recv_left_){
            recv_left_ -= n;
            return RET_INCOMPLETE;
        }else{
            switch(recvStatus_){
                case STATUS_START:{
                    if(findHttpHead()){
                        checkHttpEnd(oldsz);
                    }else if(!validateCmdHead(cmdtype)){
                        return RET_CMD_ERROR;
                    }else
                        recvStatus_ = STATUS_NO_HTTP_BODY;
                    break;}
                case STATUS_NO_HTTP_BODY:
                    return RET_COMPLETE;    //在ExportCmdData的时候设置recvStatus_
                case STATUS_HTTP:{
                    checkHttpEnd(oldsz);
                    break;}
                case STATUS_HTTP_HEAD:{
                    if(!validateCmdHead(cmdtype))
                        return RET_CMD_ERROR;
                    recvStatus_ = STATUS_HTTP_BODY;
                    break;}
                case STATUS_HTTP_BODY:
                    return RET_COMPLETE;    //在ExportCmdData的时候设置recvStatus_
                default:
                    return RET_ERROR;
            }
            return RET_CONTINUE;
        }
    }else if(!n){   //peer closed
        return RET_CLOSED;
    }else if(n < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)){   //no data
        return RET_INCOMPLETE;
    }else           //error
        return RET_ERROR;
}

int CCmdSock::SendCommand(const buffer_type & data,U32 timeoutMs)
{
    LOCAL_LOGGER(logger,"CCmdSock::SendCommand");
    if(!IsValid()){
        ERROR("fd invalid, return RET_ERROR");
        return RET_ERROR;
    }
    DEBUG("send to "<<ToString()<<" data="<<Tools::DumpHex(data));
    guard_type g(send_lock_);
    if(SendData(data,timeoutMs))
        return RET_COMPLETE;
    return RET_ERROR;
}

bool CCmdSock::PutSendData(buffer_type & data,bool end)
{
    guard_type g(send_lock_);
    if(end){
        send_data_.push_back(buffer_type());
        send_data_.back().swap(data);
    }else{
        send_data_.push_front(buffer_type());
        send_data_.front().swap(data);
    }
    if(!setSendFlag_){
        setSendFlag_ = true;
        return false;
    }
    return true;
}

bool CCmdSock::GetSendData(buffer_type & data,bool sendflag)
{
    guard_type g(send_lock_);
    if(send_data_.empty()){
        setSendFlag_ = sendflag;
        return false;
    }
    data.swap(send_data_.front());
    send_data_.pop_front();
    return true;
}

void CCmdSock::ExportCmdData(CCmdBuf & cmdbuf){
    cmdbuf.Buf.swap(recv_data_);
    cmdbuf.UseHttp = (recvStatus_ == STATUS_HTTP_BODY);
    ResetRecv();
}

void CCmdSock::ResetRecv()
{
    recv_data_.clear();
    recvStatus_ = STATUS_START;
    recv_left_ = QCmdBase::HEAD_LEN;
}

bool CCmdSock::findHttpHead() const
{
    const char HTTP_SIGN[] = {'P','O','S','T'};
    return !memcmp(&recv_data_[0],HTTP_SIGN,sizeof HTTP_SIGN);
}

size_t CCmdSock::findHttpEnd(size_t from) const
{
    const char HTTP_END[] = {'\r','\n','\r','\n'};
    if(from >= sizeof HTTP_END)
        from -= sizeof HTTP_END;
    assert(from < recv_data_.size());
    ssize_t n = Tools::StringMatch(&recv_data_[from],recv_data_.size() - from
        ,HTTP_END,sizeof HTTP_END);
    return (n < 0 ? 0 : n + sizeof HTTP_END + from);
}

void CCmdSock::checkHttpEnd(size_t oldsz)
{
    size_t cmd_head = findHttpEnd(oldsz);
    if(cmd_head){
        recv_data_.erase(recv_data_.begin(),recv_data_.begin() + cmd_head);
        assert(recv_data_.size() < QCmdBase::HEAD_LEN);
        recv_left_ = QCmdBase::HEAD_LEN - recv_data_.size();
        recvStatus_ = STATUS_HTTP_HEAD;
    }else{
        recv_left_ = QCmdBase::HEAD_LEN;
        recvStatus_ = STATUS_HTTP;
    }
}

bool CCmdSock::validateCmdHead(int cmdtype)
{
    LOCAL_LOGGER(logger,"CCmdSock::validateCmdHead");
    CInByteStream ds(recv_data_);
    QCmdBase head;
    if(!head.DecodeParam(ds)){
        WARN("data format error, command head="<<head.ToStringHelp()
            <<", data buffer="<<Tools::DumpVal(recv_data_));
        return false;
    }else if(cmdtype != -1 && int(head.CmdType()) != cmdtype){
        WARN("expect cmdtype="<<ICommand::CommandName(cmdtype)<<" but recved cmd head="
            <<head.ToString()<<", data buffer="<<Tools::DumpVal(recv_data_));
        return false;
    }
    recv_left_ = head.CmdBodyLength();
    if(recv_left_ > QCmdBase::MaxCmdLength){
        WARN("cmd body len="<<recv_left_<<" is invalid for MaxCmdLength="
            <<QCmdBase::MaxCmdLength);
        return false;
    }
    return true;
}

NS_SERVER_END

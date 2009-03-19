#include <fstream>
#include <common/Logger.h>
#include "AcceptServer.h"

NS_SERVER_BEGIN

CAcceptServer::CAcceptServer(CMainServer & mainServer)
    : CThreadPool(1,mainServer.acceptServerStatckSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , stats_(mainServer.serverStatsOn_ ? new __Stats : 0)
{}

CAcceptServer::~CAcceptServer()
{
    if(stats_)
        delete stats_;
}

void CAcceptServer::Init(const __Config & config)
{
    ipstr_ = config.GetString("server.tcp.listen.ip");
    port_ = config.GetString("server.tcp.listen.port","9530");
}

void CAcceptServer::Reconfig(const __Config & config)
{}

void CAcceptServer::ShowConfig(std::ofstream & file) const
{
    using namespace std;
    if(!file.is_open())
        return;
    file<<"\nAcceptServer :\n"
        <<"  ipstr_ = "<<ipstr_<<endl
        <<"  port_ = "<<port_<<endl
        ;
}

int CAcceptServer::doIt()
{
    LOCAL_LOGGER(logger,"CAcceptServer::doIt");
    prepare();
    for(__SockPtr client(__CmdSock::GetObject());;client = __CmdSock::GetObject()){
        if(listen_.Accept(*client) == CListenSocket::RET_ERROR){
            int eno = errno;
            ERROR("listen_="<<listen_.ToString()<<" accept error"
                <<Tools::ErrorMsg(eno));
            if(eno == EMFILE)   //too many open files
                sleep(1);
            continue;
        }
        DEBUG("new client arrived "<<Tools::ToStringPtr(client));
        client->SetLinger();
        client->SetBlock(false);
        int fd = client->FD();
        DEBUG("set fdSockMap_["<<fd<<"]="<<Tools::ToStringPtr(client)
            <<" and push into addingFdQue_");
        fdSockMap_.SetSock(fd,client);
        if(!addingFdQue_.Push(__FdEvent(fd,__FdEvent::EVENT_READ))){
            ERROR("push fd="<<fd<<" into addingFdQue_ failed, close it");
            fdSockMap_.SetSock(fd,0);
        }else if(stats_)
            stats_->Put(1);
    }
    FATAL_COUT("thread quit");
    return 0;
}

void CAcceptServer::prepare()
{
    LOCAL_LOGGER(logger,"CAcceptServer::prepare");
    CSockAddr serverAddr;
    if(!serverAddr.SetAddr(ipstr_,port_)){
        FATAL_COUT("server ip="<<ipstr_<<",port_="<<port_<<" error"
            <<CSockAddr::ErrMsg());
    }
    INFO("listen server addr="<<serverAddr.ToString());
    if(!listen_.Listen(serverAddr,true)){
        FATAL_COUT("listen server addr="<<serverAddr.ToString()<<" error"
            <<CSocket::ErrMsg());
    }
}

NS_SERVER_END

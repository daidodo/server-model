#include <iostream>
#include <fstream>
#include <Logger.h>
#include "frame/AcceptServer.h"
#include "frame/EpollServer.h"
#include "frame/PollServer.h"
#include "frame/TcpServer.h"
#include "CmdHandler.h"
#include "StatsServer.h"
#include "MainServer.h"

NS_SERVER_BEGIN

CMainServer::CMainServer()
    : acceptServer_(0)
    , epollServer_(0)
    , pollServer_(0)
    , tcpServer_(0)
    , cmdHandler_(0)
    , statsServer_(0)
    , eventFdQue_(0)
{}

CMainServer::~CMainServer()
{
    delete [] eventFdQue_;
    delete acceptServer_;
    if(epollServer_)
        delete epollServer_;
    if(pollServer_)
        delete pollServer_;
    delete tcpServer_;
    delete cmdHandler_;
    if(statsServer_)
        delete statsServer_;
}

void CMainServer::Init(const char * serverconf)
{
    assert(!acceptServer_ && !epollServer_ && !pollServer_ && !tcpServer_ && !cmdHandler_ && !statsServer_);
    LOCAL_LOGGER(logger,"CMainServer::Init");
    ASSERT(serverconf,"serverconf is null");
    configFile_ = serverconf;
    __Config config;
    if(!config.Load(configFile_.c_str())){
        FATAL_COUT("read config file="<<configFile_<<" error");
    }
    showConfigFile_ = config.GetString("show.config.file.name","show_config.txt");
    QCmdBase::MaxCmdLength = config.GetInt("tcp.cmd.max.length",1024,64);
    UdpQCmdBase::MaxCmdLength = config.GetInt("udp.cmd.max.length",1024,64);
    useEpoll_ = config.GetInt("use.epoll.server");
    serverStatsOn_ = config.GetInt("server.stats.on");
    //thread statck size
    acceptServerStatckSz_ = config.GetInt("accept.server.stack.size",16 << 10,4 << 10);
    pollServerStackSz_ = config.GetInt("poll.server.stack.size",16 << 10,4 << 10);
    epollServerStackSz_ = config.GetInt("epoll.server.stack.size",16 << 10,4 << 10);
    tcpServerStackSz_ = config.GetInt("tcp.server.stack.size",16 << 10,4 << 10);
    cmdHandlerStackSz_ = config.GetInt("command.handler.stack.size",16 << 10,4 << 10);
    statsServerStackSz_ = config.GetInt("server.stats.stack.size",16 << 10,4 << 10);
    //thread count
    tcpServerThreadCount_ = config.GetInt("tcp.server.thread.count",4,1);
    //business

    //service
    eventFdQue_ = new __FdEventQue[tcpServerThreadCount_];
    acceptServer_ = new CAcceptServer(*this);
    if(useEpoll_)
        epollServer_ = new CEpollServer(*this);
    else
        pollServer_ = new CPollServer(*this);
    tcpServer_ = new CTcpServer(*this);
    cmdHandler_ = new CCmdHandler(*this);
    if(serverStatsOn_)
        statsServer_ = new CStatsServer(*this);
    acceptServer_->Init(config);
    if(epollServer_)
        epollServer_->Init(config);
    if(pollServer_)
        pollServer_->Init(config);
    tcpServer_->Init(config);
    cmdHandler_->Init(config);
    if(statsServer_)
        statsServer_->Init(config);
}

void CMainServer::Reconfig()
{
    using namespace std;
    assert(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_);
    __Config config;
    if(!config.Load(configFile_.c_str())){
        cerr<<"read config file="<<configFile_<<" error";
    }else{
        QCmdBase::MaxCmdLength = config.GetInt("tcp.cmd.max.length",1024,64);
        UdpQCmdBase::MaxCmdLength = config.GetInt("udp.cmd.max.length",1024,64);
        acceptServer_->Reconfig(config);
        if(epollServer_)
            epollServer_->Reconfig(config);
        if(pollServer_)
            pollServer_->Reconfig(config);
        tcpServer_->Reconfig(config);
        cmdHandler_->Reconfig(config);
        if(statsServer_)
            statsServer_->Reconfig(config);
    }
}

void CMainServer::ShowConfig(std::string verInfo) const
{
    using namespace std;
    assert(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_);
    ofstream file(showConfigFile_.c_str());
    if(!file.is_open()){
        cerr<<"show config error\n";
        return;
    }
    file<<verInfo<<"\n\n"
        <<"MainServer :\n"
        <<"  configFile_ = "<<configFile_<<endl
        <<"  showConfigFile_ = "<<showConfigFile_<<endl
        <<"  TcpMaxCmdLength = "<<QCmdBase::MaxCmdLength<<endl
        <<"  UdpMaxCmdLength = "<<UdpQCmdBase::MaxCmdLength<<endl
        <<"  useEpoll_ = "<<useEpoll_<<endl
        <<"  serverStatsOn_ = "<<serverStatsOn_<<endl
        //thread statck size
        <<"  acceptServerStatckSz_ = "<<acceptServerStatckSz_<<endl
        <<"  pollServerStackSz_ = "<<pollServerStackSz_<<endl
        <<"  epollServerStackSz_ = "<<epollServerStackSz_<<endl
        <<"  tcpServerStackSz_ = "<<tcpServerStackSz_<<endl
        <<"  cmdHandlerStackSz_ = "<<cmdHandlerStackSz_<<endl
        <<"  statsServerStackSz_ = "<<statsServerStackSz_<<endl
        //thread count
        <<"  tcpServerThreadCount_ = "<<tcpServerThreadCount_<<endl
        //business
        ;
    acceptServer_->ShowConfig(file);
    if(epollServer_)
        epollServer_->ShowConfig(file);
    if(pollServer_)
        pollServer_->ShowConfig(file);
    tcpServer_->ShowConfig(file);
    cmdHandler_->ShowConfig(file);
    if(statsServer_)
        statsServer_->ShowConfig(file);
}

void CMainServer::StartServer()
{
    LOCAL_LOGGER(logger,"CMainServer::StartServer");
    ASSERT(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_,
        "acceptServer_="<<acceptServer_<<",epollServer_="<<epollServer_<<",pollServer_="
        <<pollServer_<<",tcpServer_="<<tcpServer_<<",cmdHandler_="<<cmdHandler_);
    int ret = 0;
    ret = acceptServer_->StartThreads("Accept Server");
    if(ret <= 0){
        FATAL_COUT("start accept server error ret="<<ret);
    }
    if(epollServer_){
        ret = epollServer_->StartThreads("Epoll Server");
        if(ret <= 0){
            FATAL_COUT("start epoll server error ret="<<ret);
        }
    }
    if(pollServer_){
        ret = pollServer_->StartThreads("Poll Server");
        if(ret <= 0){
            FATAL_COUT("start poll server error ret="<<ret);
        }
    }
    ret = tcpServer_->StartThreads("Tcp Server");
    if(ret <= 0){
        FATAL_COUT("start tcp server error ret="<<ret);
    }
    ret = cmdHandler_->StartThreads("Command Handler");
    if(ret <= 0){
        FATAL_COUT("start command handler error ret="<<ret);
    }
    if(statsServer_){
        ret = statsServer_->StartThreads("Statistic Server");
        if(ret <= 0){
            FATAL_COUT("start stats server error ret="<<ret);
        }
    }
    acceptServer_->WaitAll();
    if(epollServer_)
        epollServer_->WaitAll();
    if(pollServer_)
        pollServer_->WaitAll();
    tcpServer_->WaitAll();
    cmdHandler_->WaitAll();
    if(statsServer_)
        statsServer_->WaitAll();
}

NS_SERVER_END

#include <common/impl/Config.h>
#include <common/Logger.h>
#include <server/frame/AcceptServer.h>
#include <server/frame/EpollServer.h>
#include <server/frame/PollServer.h>
#include <server/frame/TcpServer.h>
#include <server/CmdHandler.h>
#include "StatsServer.h"

NS_SERVER_BEGIN

CStatsServer::CStatsServer(CMainServer & mainServer)
    : CThreadPool(1,mainServer.statsServerStackSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , removeFdQue_(mainServer.removeFdQue_)
    , eventFdQue_(mainServer.eventFdQue_)
    , EVENT_QUE_SZ_(mainServer.tcpServerThreadCount_)
    , queryCmdQue_(mainServer.queryCmdQue_)
    , acceptServer_(mainServer.acceptServer_)
    , epollServer_(mainServer.epollServer_)
    , pollServer_(mainServer.pollServer_)
    , tcpServer_(mainServer.tcpServer_)
    , cmdHandler_(mainServer.cmdHandler_)
{}

void CStatsServer::Init(const __Config & config)
{
    serverStatusFile_           = config.GetString("server.stats.file.name");
    serverStatusTimestamp_      = config.GetString("server.stats.file.name.timestamp");
    serverStatusTimeInterval_   = config.GetInt("server.stats.time.interval",30,5);
}

void CStatsServer::Reconfig(const __Config & config)
{
    serverStatusTimeInterval_   = config.GetInt("server.stats.time.interval",30,5);
}

void CStatsServer::ShowConfig(std::ofstream & file) const
{
    using namespace std;
    if(!file.is_open())
        return;
    file<<"\nStatsServer :\n"
        <<"  serverStatusFile_ = "<<serverStatusFile_<<endl
        <<"  serverStatusTimestamp_ = "<<serverStatusTimestamp_<<endl
        <<"  serverStatusTimeInterval_ = "<<serverStatusTimeInterval_<<endl;
}

int CStatsServer::doIt()
{
    for(;;){
        sleep(serverStatusTimeInterval_);
        writeStats();
    }
    return 0;
}

void CStatsServer::writeStats()
{
    assert(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_);
    LOCAL_LOGGER(logger,"CStatsServer::writeStats");
    std::string fname = serverStatusFile_;
    if(!serverStatusTimestamp_.empty())
        fname += Tools::TimeString(time(0),serverStatusTimestamp_.c_str());
    std::ofstream outf(fname.c_str(),std::ios_base::app);
    if(!outf.is_open()){
        ERROR("cannot open server status file="<<serverStatusFile_);
        return;
    }
    outf<<"Time : "<<Tools::TimeString(time(0));
    //status
    U32 v[7] = {0};
    acceptServer_->stats_->Get(v[0]);
    if(epollServer_)
        epollServer_->stats_->Get(v[1],v[2],v[3],v[4]);
    else if(pollServer_)
        pollServer_->stats_->Get(v[1],v[2],v[3],v[4]);
    outf<<"\n  New/Read/Write : "
        <<v[0]<<"/"<<v[1]<<"/"<<v[2];
    outf<<"\n  Error/Timeout : "
        <<v[3]<<"/"<<v[4];
    tcpServer_->stats_->Get(v[0],v[1],v[2],v[3],v[4],v[5],v[6]);
    outf<<"\n  PeerClose/RecvErr : "
        <<v[0]<<"/"<<v[1];
    outf<<"\n  HeadErr/BodyErr : "
        <<v[2]<<"/"<<v[3];
    outf<<"\n  SendCmd/SendErr/SendUnfinish : "
        <<v[4]<<"/"<<v[5]<<"/"<<v[6];
    outf<<"\n  Online : "<<fdSockMap_.Size();
    //queues
    outf<<"\n  addingFdQue_.Size : "
        <<addingFdQue_.Size()<<"/"<<addingFdQue_.ResetTopSize();
    outf<<"\n  removeFdQue_.Size : "
        <<removeFdQue_.Size()<<"/"<<removeFdQue_.ResetTopSize();
    for(int i = 0;i < EVENT_QUE_SZ_;++i)
        outf<<"\n  eventFdQue_["<<i<<"].Size : "
            <<eventFdQue_[i].Size()<<"/"<<eventFdQue_[i].ResetTopSize();
    outf<<"\n  queryCmdQue_.Size : "
        <<queryCmdQue_.Size()<<"/"<<queryCmdQue_.ResetTopSize();
    //threads
    outf<<"\n  tcpServer_.ActiveCount : "
        <<tcpServer_->ActiveCount()<<"/"<<tcpServer_->ResetActiveMax();
    outf<<"\n  cmdHandler_.ActiveCount : "
        <<cmdHandler_->ActiveCount()<<"/"<<cmdHandler_->ThreadCount();
    //business

    //commands
    if(cmdHandler_->stats_){
        typedef __CmdStats::container_type __Cont;
        __CmdStats & cmdStats = cmdHandler_->stats_->cmdStats_;
        __Cont tmp;
        cmdStats.Lock();
        cmdStats.swap(tmp);
        cmdStats.Unlock();
        for(__Cont::iterator it = tmp.begin();it != tmp.end();++it)
            outf<<"\n  "<<QCmdBase::CommandName(it->first)<<" : "
                <<it->second.ToString();
    }
    outf<<std::endl;
}

NS_SERVER_END

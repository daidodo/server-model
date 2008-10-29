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
    __DZ_STRING timestamp       = config.GetString("server.stats.file.name.timestamp",".%y%m%d%H%M%S");
    serverStatusFile_           = config.GetString("server.stats.file.name");
    serverStatusTimeInterval_   = config.GetInt("server.stats.time.interval",30,5);
    serverStatusFile_ += Tools::TimeString(time(0),timestamp);
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
        <<"  serverStatusTimeInterval_ = "<<serverStatusTimeInterval_<<endl;
}

int CStatsServer::doIt()
{
    writeHead();
    for(;;){
        sleep(serverStatusTimeInterval_);
        writeStats();
    }
    LOCAL_LOGGER(logger,"CStatsServer::doIt");
    FATAL_COUT("thread quit");
    return 0;
}

void CStatsServer::writeHead() const
{
    assert(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_);
    LOCAL_LOGGER(logger,"CStatsServer::writeHead");
    DEBUG("open server stats file "<<serverStatusFile_);
    std::ofstream outf(serverStatusFile_.c_str());
    if(!outf.is_open()){
        ERROR("cannot open server status file="<<serverStatusFile_);
        return;
    }
    outf<<"Time"
        //stats
        <<"\t"<<"New/Read/Write"
        <<"\t"<<"Error/Timeout"
        <<"\t"<<"PeerClose/RecvErr"
        <<"\t"<<"HeadErr/BodyErr"
        <<"\t"<<"SendCmd/SendErr/SendUnfinish"
        <<"\t"<<"Online"
        //queue
        <<"\t"<<"addingFdQue_.Size()"
        <<"\t"<<"removeFdQue_.Size()";
    for(int i = 0;i < EVENT_QUE_SZ_;++i)
        outf<<"\t"<<"eventFdQue_["<<i<<"].Size()";
    outf<<"\t"<<"queryCmdQue_.Size()"
        //thread
        <<"\t"<<"tcpServer_.ActiveCount()"
        <<"\t"<<"cmdHandler_.ActiveCount()"
        //business
        //command stats
        <<"\t"<<"CommandStats(Name,Succ,Fail,TimeUs,AveTimeUs)"
        <<std::endl;
}

void CStatsServer::writeStats()
{
    assert(acceptServer_ && (epollServer_ || pollServer_) && tcpServer_ && cmdHandler_);
    LOCAL_LOGGER(logger,"CStatsServer::writeStats");
    DEBUG("open server stats file "<<serverStatusFile_<<" and write");
    std::ofstream outf(serverStatusFile_.c_str(),std::ios_base::app);
    if(!outf.is_open()){
        ERROR("cannot open server status file="<<serverStatusFile_);
        return;
    }
    outf<<Tools::TimeString(time(0));
    //stats
    U32 v[7] = {0};
    acceptServer_->stats_->Get(v[0]);
    if(epollServer_)
        epollServer_->stats_->Get(v[1],v[2],v[3],v[4]);
    else if(pollServer_)
        pollServer_->stats_->Get(v[1],v[2],v[3],v[4]);
    outf<<"\t"<<v[0]<<"/"<<v[1]<<"/"<<v[2];     // New/Read/Write
    outf<<"\t"<<v[3]<<"/"<<v[4];                // Error/Timeout
    tcpServer_->stats_->Get(v[0],v[1],v[2],v[3],v[4],v[5],v[6]);
    outf<<"\t"<<v[0]<<"/"<<v[1];                // PeerClose/RecvErr
    outf<<"\t"<<v[2]<<"/"<<v[3];                // HeadErr/BodyErr
    outf<<"\t"<<v[4]<<"/"<<v[5]<<"/"<<v[6];     // SendCmd/SendErr/SendUnfinish
    outf<<"\t"<<fdSockMap_.Size();
    //queue
    outf<<"\t"<<addingFdQue_.Size()<<"/"<<addingFdQue_.ResetTopSize();
    outf<<"\t"<<removeFdQue_.Size()<<"/"<<removeFdQue_.ResetTopSize();
    for(int i = 0;i < EVENT_QUE_SZ_;++i)
        outf<<"\t"<<eventFdQue_[i].Size()<<"/"<<eventFdQue_[i].ResetTopSize();
    outf<<"\t"<<queryCmdQue_.Size()<<"/"<<queryCmdQue_.ResetTopSize();
    //thread
    outf<<"\t"<<tcpServer_->ActiveCount()<<"/"<<tcpServer_->ResetActiveMax();
    outf<<"\t"<<cmdHandler_->ActiveCount()<<"/"<<cmdHandler_->ResetActiveMax();
    //business
    //command stats
    if(cmdHandler_->stats_){
        typedef __CmdStats::container_type      __Cont;
        typedef __Cont::mapped_type::third_type __Time;
        __CmdStats & cmdStats = cmdHandler_->stats_->cmdStats_;
        __Cont tmp;
        cmdStats.Lock();
        cmdStats.swap(tmp);
        cmdStats.Unlock();
        for(__Cont::iterator it = tmp.begin();it != tmp.end();++it){
            outf<<"\t"<<QCmdBase::CommandName(it->first)<<","<<it->second.first
                <<","<<it->second.second<<","<<it->second.third<<",";
            __Time ave = it->second.first + it->second.second;
            if(ave)
                ave = it->second.third / ave;
            outf<<ave;
        }
    }
    outf<<std::endl;
}

NS_SERVER_END

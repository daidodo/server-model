#include <iostream>
#include <common/Logger.h>
#include <common/Tools.h>
#include "CmdHandler.h"

NS_SERVER_BEGIN

CCmdHandler::CCmdHandler(CMainServer & mainServer)
    : __MyBase(mainServer.queryCmdQue_,mainServer.cmdHandlerStackSz_)
    , fdSockMap_(mainServer.fdSockMap_)
    , addingFdQue_(mainServer.addingFdQue_)
    , eventFdQue_(mainServer.eventFdQue_)
    , EVENT_QUE_SZ_(mainServer.tcpServerThreadCount_)
    , useEpoll_(mainServer.useEpoll_)
    , stats_(mainServer.serverStatsOn_ ? new __Stats : 0)
    //business

{}

CCmdHandler::~CCmdHandler()
{
    if(stats_)
        delete stats_;
}
        
void CCmdHandler::Init(const __Config & config)
{
    if(stats_){
        CCommandStats::SetCmdStats(&stats_->cmdStats_);
        CCommandStats::TimeStats(config.GetInt("server.stats.cmd.time.on"));
    }
    //business
}

void CCmdHandler::Reconfig(const __Config & config)
{
    if(stats_)
        CCommandStats::TimeStats(config.GetInt("server.stats.cmd.time.on"));
    //business
}

void CCmdHandler::ShowConfig(std::ofstream & file) const
{
    using namespace std;
    if(!file.is_open())
        return;
    file<<"\nCmdHandler :\n"
        <<"  CCommandStats::TimeStats="<<CCommandStats::TimeStats()<<endl
        //business
        ;
}

int CCmdHandler::StartThreads(__DZ_STRING name)
{
    //business
    return __MyBase::StartThreads(name);
}

void CCmdHandler::WaitAll()
{
    //business
    __MyBase::WaitAll();
}

void CCmdHandler::doIt(__Job & job)
{
 //   LOCAL_LOGGER(logger,"CCmdHandler::doIt");
 //   ASSERT(eventFdQue_ && EVENT_QUE_SZ_ > 0,"eventFdQue_="<<eventFdQue_
 //       <<", EVENT_QUE_SZ_="<<EVENT_QUE_SZ_<<" invalid");
	//for(__CmdTriple triple;;){
	//	if(!queryCmdQue_.Pop(triple)){
	//		WARN("pop from queryCmdQue_ failed");
	//		continue;
	//	}
	//	__Active active(Cnt());	//活跃线程计数
 //       process(triple);
 //   }
 //   FATAL_COUT("thread quit");
	//return 0;
    process(job);
}

void CCmdHandler::readyForSend(const RCmdBase & cmd,__Buf & respdata)
{
    COutByteStream ds;
    cmd.Encode(ds);
    __Buf data;
    ds.ExportData(data);
    //http
    if(cmd.UseHttp()){
        __DZ_OSTRINGSTREAM oss;
        oss<<"HTTP/1.1 200 OK\r\nContent-Length: "<<data.size()
            <<"\r\nContent-Type: Application/octet-stream\r\nConnection: Close\r\n\r\n";
        __DZ_STRING http = oss.str();
        respdata.reserve(respdata.size() + http.length() + data.size());
        respdata.insert(respdata.end(),http.begin(),http.end());
    }//encode
    respdata.insert(respdata.end(),data.begin(),data.end());
}

NS_SERVER_END

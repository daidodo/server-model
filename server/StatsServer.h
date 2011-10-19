#ifndef DOZERG_STATS_SERVER_H_20080121
#define DOZERG_STATS_SERVER_H_20080121

#include <common/Threads.h>
#include <server/MainServer.h>

NS_SERVER_BEGIN

class CStatsServer : public CThreadPool
{
    //typedefs:
    typedef CMainServer::__Config       __Config;
    typedef CMainServer::__FdQue        __FdQue;
    typedef CMainServer::__FdEventQue   __FdEventQue;
    typedef CMainServer::__FdSockMap    __FdSockMap;
    typedef CMainServer::__QueryCmdQue  __QueryCmdQue;
    //members:
    __DZ_STRING             serverStatusFile_;          //服务器统计文件名,不能重新配置
    __DZ_STRING             serverStatusTimestamp_;     //服务器统计文件名的时间戳格式,不能重新配置
    int                     serverStatusTimeInterval_;  //s,服务器统计间隔
    __FdSockMap &           fdSockMap_;
    __FdEventQue &          addingFdQue_;
    __FdQue &               removeFdQue_;
    __FdEventQue * const    eventFdQue_;
    const int               EVENT_QUE_SZ_;
    __QueryCmdQue &         queryCmdQue_;
    CAcceptServer * const   acceptServer_;
    CEpollServer * const    epollServer_;
    CPollServer * const     pollServer_;
    CTcpServer * const      tcpServer_;
    CCmdHandler * const     cmdHandler_;
    //business

public:
    explicit CStatsServer(CMainServer & mainServer);
    void Init(const __Config & config);
    void Reconfig(const __Config & config);     //重新读取配置文件
    void ShowConfig(std::ofstream & file) const;//显示当前配置信息
protected:
	int doIt();
private:
    void writeStats();
};

NS_SERVER_END

#endif

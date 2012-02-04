#ifndef DOZERG_STATS_SERVER_H_20080121
#define DOZERG_STATS_SERVER_H_20080121

#include <string>
#include <Threads.h>
#include "MainServer.h"

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
    std::string             serverStatusFile_;          //������ͳ���ļ���,������������
    std::string             serverStatusTimestamp_;     //������ͳ���ļ�����ʱ�����ʽ,������������
    int                     serverStatusTimeInterval_;  //s,������ͳ�Ƽ��
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
    void Reconfig(const __Config & config);     //���¶�ȡ�����ļ�
    void ShowConfig(std::ofstream & file) const;//��ʾ��ǰ������Ϣ
protected:
	int doIt();
private:
    void writeStats();
};

NS_SERVER_END

#endif

#ifndef DOZERG_COMMAND_HANDLER_H_20071220
#define DOZERG_COMMAND_HANDLER_H_20071220

#include <common/Threads.h>
#include <server/MainServer.h>
#include <common/LockHashMap.h>

NS_SERVER_BEGIN

class CCmdHandler:public CThreadManager<CMainServer::__QueryCmdQue>
{
//typedefs:
    typedef CThreadManager<CMainServer::__QueryCmdQue>   __MyBase;
    typedef __MyBase::__Queue           __CmdQue;
    typedef CMainServer::__Config       __Config;
    typedef CMainServer::__CmdSock      __CmdSock;
    typedef __CmdSock::buffer_type      __Buf;
    typedef CMainServer::__SockPtr      __SockPtr;
    typedef CMainServer::__FdEvent      __FdEvent;
    typedef CMainServer::__FdEventQue   __FdEventQue;
    typedef CMainServer::__FdSockMap    __FdSockMap;
    typedef __MyBase::__Job             __Job;
    struct __Stats{
        __CmdStats cmdStats_;
    };
    //business:

//members:
    __FdSockMap &           fdSockMap_;
    __FdEventQue &          addingFdQue_;
    __FdEventQue * const    eventFdQue_;
    const int               EVENT_QUE_SZ_;
    const bool              useEpoll_;
public:
    __Stats * const         stats_;
    //business:

//functions:
    //���º�����"CmdHandler.cpp"�ڶ���
    explicit CCmdHandler(CMainServer & mainServer);
    ~CCmdHandler();
    void Init(const __Config & config);
    void Reconfig(const __Config & config);     //���¶�ȡ�����ļ�
    void ShowConfig(std::ofstream & file) const;//��ʾ��ǰ������Ϣ
    virtual int StartThreads(__DZ_STRING name,int thread_count = 0);
    virtual void WaitAll();
protected:
	void doIt(__Job & job);
private:
    void readyForSend(const RCmdBase & cmd,__Buf & respdata);
    void readyForSend(const UdpRCmdBase & cmd,__Buf & respdata);
    //���º�����"CmdProcess.cpp"�ڶ���
    void process(const __Job & cmdTriple);
    //ADD NEW COMMAND PROCESS HERE
    void processCmd(CQueryCmd & cmd,__Buf & respdata,CCommandStats & stats);
    //business:

};

NS_SERVER_END

#endif

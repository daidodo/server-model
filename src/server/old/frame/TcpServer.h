#ifndef DOZERG_TCP_SERVER_H_20080909
#define DOZERG_TCP_SERVER_H_20080909

#include <Threads.h>
#include "../MainServer.h"

NS_SERVER_BEGIN

class CTcpServer : public CThreadPool
{
    //typedefs:
    typedef CMainServer::__Config           __Config;
    typedef CMainServer::__CmdSock          __CmdSock;
    typedef __CmdSock::buffer_type          __Buf;
    typedef CMainServer::__SockPtr          __SockPtr;
    typedef CMainServer::__FdQue            __FdQue;
    typedef CMainServer::__FdEvent          __FdEvent;
    typedef CMainServer::__FdEventQue       __FdEventQue;
    typedef CMainServer::__FdSockMap        __FdSockMap;
    typedef CMainServer::__CmdTriple        __CmdTriple;
    typedef CMainServer::__QueryCmdQue      __QueryCmdQue;
    struct __Stats : public CSpinLock
    {
        typedef CGuard<CSpinLock>  guard_type;
        U32 closeCount_;        //�ͻ��������ر�ͳ��
        U32 recvErrCount_;      //���մ���ͳ��
        U32 headErrCount_;      //����ͷ����ͳ��
        U32 bodyErrCount_;      //���������ͳ��
        U32 sendCmdCount_;      //���ͳɹ�������ͳ��
        U32 sendErrCount_;      //���ʹ���ͳ��
        U32 sendUnfinishCount_; //����δ���ͳ��
        __Stats()
            : closeCount_(0)
            , recvErrCount_(0)
            , headErrCount_(0)
            , bodyErrCount_(0)
            , sendCmdCount_(0)
            , sendErrCount_(0)
            , sendUnfinishCount_(0)
        {}
        void Put(U32 cc,U32 rc,U32 hc,U32 bc,U32 sc,U32 se,U32 su){
            guard_type g(*this);
            closeCount_ += cc;
            recvErrCount_ += rc;
            headErrCount_ += hc;
            bodyErrCount_ += bc;
            sendCmdCount_ += sc;
            sendErrCount_ += se;
            sendUnfinishCount_ += su;
        }
        void Get(U32 & cc,U32 & rc,U32 & hc,U32 & bc,U32 & sc,U32 & se,U32 & su){
            guard_type g(*this);
            cc = closeCount_;
            rc = recvErrCount_;
            hc = headErrCount_;
            bc = bodyErrCount_;
            sc = sendCmdCount_;
            se = sendErrCount_;
            su = sendUnfinishCount_;
            closeCount_ = 0;
            recvErrCount_ = 0;
            headErrCount_ = 0;
            bodyErrCount_ = 0;
            sendCmdCount_ = 0;
            sendErrCount_ = 0;
            sendUnfinishCount_ = 0;
        }
    };
    //members:
    __FdSockMap &           fdSockMap_;
    __FdEventQue &          addingFdQue_;
    __FdQue &               removeFdQue_;
    __FdEventQue * const    eventFdQue_;
    const int               EVENT_QUE_SZ_;
    __QueryCmdQue &         queryCmdQue_;
    const bool              useEpoll_;
    CLockInt<int>           index_;     //��ÿ���̷߳���eventFdQue_����
public:
    __Stats * const         stats_;
    //functions:
    explicit CTcpServer(CMainServer & mainServer);
    ~CTcpServer();
    void Init(const __Config & config){}
    void Reconfig(const __Config & config){}        //���¶�ȡ�����ļ�
    void ShowConfig(std::ofstream & file) const{}   //��ʾ��ǰ������Ϣ
protected:
	int doIt();
private:
    __FdEventQue & getEventQue();
    //�����Ƿ�������������
    bool sendCmdData(__CmdSock & sock,U32 & sc,U32 & se,U32 & su) const;
    //�����Ƿ�����
    bool recvCmdData(int fd,__SockPtr & pSock,U32 & cc,U32 & rc,U32 & hc,U32 & bc);
};

NS_SERVER_END

#endif

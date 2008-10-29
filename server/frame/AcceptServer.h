#ifndef DOZERG_ACCEPT_SERVER_H_20080908
#define DOZERG_ACCEPT_SERVER_H_20080908

#include <common/Threads.h>
#include <server/MainServer.h>

NS_SERVER_BEGIN

class CAcceptServer : public CThreadPool
{
    //typedefs:
    typedef CMainServer::__Config       __Config;
    typedef CMainServer::__CmdSock      __CmdSock;
    typedef CMainServer::__SockPtr      __SockPtr;
    typedef CMainServer::__FdEvent      __FdEvent;
    typedef CMainServer::__FdEventQue   __FdEventQue;
    typedef CMainServer::__FdSockMap    __FdSockMap;
    struct __Stats : public CSpinLock
    {
        typedef CGuard<CSpinLock>   guard_type;
        U32 clientCount_;   //�¿ͻ���ͳ��
        __Stats():clientCount_(0){}
        void Put(U32 cc){
            guard_type g(*this);
            clientCount_ += cc;
        }
        void Get(U32 & cc){
            guard_type g(*this);
            cc = clientCount_;
            clientCount_ = 0;
        }
    };
    //members:
    __FdSockMap &   fdSockMap_;
    __FdEventQue &  addingFdQue_;
    __DZ_STRING     ipstr_,port_;   //������ip�Ͷ˿�,������������
    CListenSocket   listen_;        //����socket
public:
    __Stats * const stats_;
    //functions:
    explicit CAcceptServer(CMainServer & mainServer);
    ~CAcceptServer();
    void Init(const __Config & config);
    void Reconfig(const __Config & config);     //���¶�ȡ�����ļ�
    void ShowConfig(std::ofstream & file) const;//��ʾ��ǰ������Ϣ
protected:
	int doIt();
private:
    void prepare();
};

NS_SERVER_END

#endif

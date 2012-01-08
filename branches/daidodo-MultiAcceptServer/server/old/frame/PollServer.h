#ifndef DOZERG_POLL_SERVER_H_20080917
#define DOZERG_POLL_SERVER_H_20080917

#include <Threads.h>
#include <Poll.h>
#include "../MainServer.h"

NS_SERVER_BEGIN

class CPollServer : public CThreadPool
{
    //typedefs:
    typedef CMainServer::__Config           __Config;
    typedef CMainServer::__CmdSock          __CmdSock;
    typedef CMainServer::__SockPtr          __SockPtr;
    typedef CMainServer::__FdQue            __FdQue;
    typedef CMainServer::__FdEvent          __FdEvent;
    typedef CMainServer::__FdEventQue       __FdEventQue;
    typedef CMainServer::__FdSockMap        __FdSockMap;
    typedef std::vector<__SockPtr>          __SockPtrVec;
    typedef std::vector<int>                __FdVec;
    typedef std::pair<int,U32>              __FdTime;
    typedef std::vector<__FdTime>           __FdTimeVec;
    typedef __FdEventQue::container_type    __FdEventList;
    typedef std::vector<__FdEventList>      __FdEvListVec;
    struct __Stats : public CSpinLock
    {
        typedef CGuard<CSpinLock>   guard_type;
        U32 readCount_;     //�ɶ�������ͳ��
        U32 writeCount_;    //��д������ͳ��
        U32 errorCount_;    //����������ͳ��
        U32 timeoutCount_;  //��ʱ�ͻ�����ͳ��
        __Stats()
            : readCount_(0)
            , writeCount_(0)
            , errorCount_(0)
            , timeoutCount_(0)
        {}
        void Put(U32 rc,U32 wc,U32 ec,U32 tc){
            guard_type g(*this);
            readCount_ += rc;
            writeCount_ += wc;
            errorCount_ += ec;
            timeoutCount_ += tc;
        }
        void Get(U32 & rc,U32 & wc,U32 & ec,U32 & tc){
            guard_type g(*this);
            rc = readCount_;
            wc = writeCount_;
            ec = errorCount_;
            tc = timeoutCount_;
            readCount_ = 0;
            writeCount_ = 0;
            errorCount_ = 0;
            timeoutCount_ = 0;
        }
    };
    //members:
    __FdSockMap &           fdSockMap_;
    __FdEventQue &          addingFdQue_;
    __FdQue &               removeFdQue_;
    __FdEventQue * const    eventFdQue_;
    const int               EVENT_QUE_SZ_;
    U32                     maxFdSz_;   //���fd��Ŀ,������������
    CPoll                   poll_;      //poll����
    __FdVec                 errFdVec_;  //����fd������
    __FdTimeVec             expFdVec_;  //��ʱfd������
    __FdEvListVec           eventList_; //�ɶ�дfd������
public:
    __Stats * const         stats_;
    //functions:
    explicit CPollServer(CMainServer & mainServer);
    ~CPollServer();
    void Init(const __Config & config);
    void Reconfig(const __Config & config);     //���¶�ȡ�����ļ�
    void ShowConfig(std::ofstream & file) const;//��ʾ��ǰ������Ϣ
protected:
	int doIt();
private:
    void prepare();
    U32 handleErrorFd();
    U32 handleExpiredFd();
    U32 flushEventList();
    void removeClosedFd();
    void addClient(U32 curtime);
};

NS_SERVER_END

#endif

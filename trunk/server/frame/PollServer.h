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
        U32 readCount_;     //可读请求数统计
        U32 writeCount_;    //可写请求数统计
        U32 errorCount_;    //出错请求数统计
        U32 timeoutCount_;  //超时客户端数统计
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
    U32                     maxFdSz_;   //最大fd数目,不能重新配置
    CPoll                   poll_;      //poll对象
    __FdVec                 errFdVec_;  //出错fd缓冲区
    __FdTimeVec             expFdVec_;  //超时fd缓冲区
    __FdEvListVec           eventList_; //可读写fd缓冲区
public:
    __Stats * const         stats_;
    //functions:
    explicit CPollServer(CMainServer & mainServer);
    ~CPollServer();
    void Init(const __Config & config);
    void Reconfig(const __Config & config);     //重新读取配置文件
    void ShowConfig(std::ofstream & file) const;//显示当前配置信息
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

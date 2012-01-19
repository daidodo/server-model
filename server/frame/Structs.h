#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

#include <vector>

#include <LockQueue.h>
#include <SharedPtr.h>
#include <FdMap.h>
#include <Sockets.h>

NS_SERVER_BEGIN

//event flags
typedef U32 __Events;

const __Events EVENT_CLOSE = 1 << 0;
const __Events EVENT_IN = 1 << 1;
const __Events EVENT_OUT = 1 << 2;
const __Events EVENT_RECV = 1 << 3;
const __Events EVENT_SEND = 1 << 4;
const __Events EVENT_ACCEPT = 1 << 5;
const __Events EVENT_READ = 1 << 6;
const __Events EVENT_WRITE = 1 << 7;

namespace Events{
    inline bool NeedClose(__Events ev){return (ev & EVENT_CLOSE);}
    inline bool CanInput(__Events ev){return (ev & EVENT_IN);}
    inline bool CanOutput(__Events ev){return (ev & EVENT_OUT);}
    inline bool CanRecv(__Events ev){return (ev & EVENT_RECV);}
    inline bool CanSend(__Events ev){return (ev & EVENT_SEND);}
    inline bool CanAccept(__Events ev){return (ev & EVENT_ACCEPT);}
    inline bool CanRead(__Events ev){return (ev & EVENT_READ);}
    inline bool CanWrite(__Events ev){return (ev & EVENT_WRITE);}
    inline bool NeedInput(__Events ev){
        return (ev & EVENT_RECV)
            || (ev & EVENT_ACCEPT)
            || (ev & EVENT_READ);
    }
    inline bool NeedOutput(__Events ev){
        return (ev & EVENT_SEND)
            || (ev & EVENT_WRITE);
    }
}//namespace Events

struct CFdEvent
{
    static const int & ExtractFd(const CFdEvent & fe){return fe.fd_;}
    CFdEvent(int f, __Events e)
        : fd_(f)
        , ev_(e)
    {}
    int Fd() const{return fd_;}
    __Events Events() const{return ev_;}
private:
    //members:
    int fd_;
    __Events ev_;
};

struct CCmdBase
{
    typedef std::allocator<CCmdBase> allocator_type;
    static CCmdBase * GetObject(){return 0;}
    std::string ToString() const{return "";}
};

typedef CCmdBase __CmdBase;
typedef std::vector<char> __Buffer;

struct CRecvHelper
{
    typedef std::pair<int, size_t> (*__OnRecv)(const __Buffer &);
    CRecvHelper()
        : initSz_(0)
        , needSz_(0)
    {}
    void SetInitSize(size_t sz){initSz_ = 0;}
    void AddStep(__OnRecv onRecv){steps_.push_back(onRecv);}
    bool RecvBuf(){
        return true;
    }
private:
    std::vector<__OnRecv> steps_;
    size_t initSz_;
    __Buffer recvBuf_;
    size_t needSz_;
};

struct CCmdSock
{
    typedef std::allocator<CCmdSock> allocator_type;
    //constants
    static const int RET_IO_ERROR = -2;     //io出错
    static const int RET_IO_RETRY = -1;     //io需要重试
    static const int RET_IO_COMPLETE = 0;   //io操作完成
    static const int RET_CMD_ERROR = -1;    //命令处理出错
    static const int RET_CMD_SUCC = 0;      //命令处理完成
public:
    //functions
    static CCmdSock * GetObject(){
        CCmdSock * ret = allocator_type().allocate(1);
        return new (ret) CCmdSock;
    }
    CCmdSock()
        : fileDesc_(0)
        , ev_(0)
    {}
    int Fd() const{return fileDesc_->Fd();}
    void Close(){fileDesc_->Close();}
    //获取/设置事件标志
    __Events Events() const{return ev_;}
    void Events(__Events ev){ev_ = ev;}
    //接收数据，decode cmd
    //return: 是否出错
    //解出的命令通过cmd返回
    bool RecvCmd(__CmdBase *& cmd);
    //接受新tcp客户端
    CCmdSock * Accept(){
        //CSocket * sock = 0;
        //sock->SetLinger();
        //sock->SetBlock(false);
        return 0;
    }
    //发送缓冲的数据
    //return: 是否出错
    bool SendBuffer(){return true;}
    //将缓冲的数据写入文件
    //return: 是否出错
    bool WriteData(){return true;}
    //将buf加入发送缓冲
    void AddSendBuf(__Buffer & buf){}
    //处理cmd
    void Process(__CmdBase & cmd){}
    std::string ToString() const{return "";}
private:
    //members
    IFileDesc * fileDesc_;
    __Events ev_;
};

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef __FdEventQue::container_type __FdEventList;
typedef CLockQueue<int> __FdQue;
typedef __FdQue::container_type __FdList;
typedef CCmdSock __CmdSock;
typedef CSharedPtr<__CmdSock> __SockPtr;
typedef CFdSockMap<__CmdSock, __SockPtr> __FdSockMap;
typedef Tools::CTriple<__CmdBase *, int, __SockPtr> __CmdTriple;
typedef CLockQueue<__CmdTriple> __QueryCmdQue;

typedef std::vector<__SockPtr> __SockPtrList;
typedef std::vector<int> __FdArray;

NS_SERVER_END

#endif


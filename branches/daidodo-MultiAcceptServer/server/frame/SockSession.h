#ifndef DOZERG_SOCK_SESSION_H_20120122
#define DOZERG_SOCK_SESSION_H_20120122

#include <memory>
#include <string>
#include <vector>
#include <list>

#include <FileDesc.h>
#include <SharedPtr.h>
#include <Sockets.h>
#include <LockInt.h>
#include <LockQueue.h>
#include <FdMap.h>
#include "Events.h"
#include "RecvHelper.h"

NS_SERVER_BEGIN

class CCmdSession;
typedef CCmdSession __CmdSession;

typedef CLockQueue<__CmdSession *> __QueryCmdQue;
typedef __QueryCmdQue::container_type __QueryCmdList;

class CCmdBase;
typedef CCmdBase __CmdBase;

typedef std::string __Buffer;
typedef std::list<__Buffer> __BufList;

class CSockSession
{
    typedef CMutex __LockType;
    typedef CGuard<__LockType> __Guard;
    typedef CLockInt<__Events> __LockEvents;
    typedef std::list<CSockAddr> __AddrList;
    typedef CRecvHelper __RecvHelper;
public:
    typedef std::allocator<CSockSession> allocator_type;
    //functions
    static CSockSession * GetObject(IFileDesc * fileDesc, const __RecvHelper & recvHelper){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession(fileDesc, recvHelper);
    }
    ~CSockSession();
    int Fd() const{return fileDesc_->Fd();}
    EFileDescType FileType() const{return fileDesc_->Type();}
    bool IsValid() const{return fileDesc_ && fileDesc_->IsValid();}
    void Close(){fileDesc_->Close();}
    std::string ToString() const;
    //获取recv helper
    const CRecvHelper & RecvHelper() const{return recvHelper_;}
    //获取/设置事件标志
    __Events Events() const{return ev_;}
    void Events(__Events events){ev_ = events;}
    //接受新tcp客户端
    //client: 返回成功接受的client
    //events: 返回client的下一个事件(EVENT_OUT, EVENT_IN)
    //return: true-正常; false-出错
    bool Accept(CSockSession *& client, __Events & events);
    //接收数据，decode cmd
    //cmd: 返回成功解码的cmd
    //udpClientAddr: 如果是udp连接，返回对方地址
    bool RecvTcpCmd(__CmdBase *& cmd);
    bool RecvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
    //发送缓冲区的数据
    bool SendTcpData();
    bool SendUdpData();
    //将缓冲区的数据写入文件
    bool WriteData();
    //处理cmd
    //udpClientAddr: 如果是udp连接，表示对方地址
    //return: EVENT_OUT-需要output; EVENT_IN-需要input; EVENT_CLOSE-需要close
    __Events Process(__CmdBase & cmd, CSockAddr & udpClientAddr);
    //释放cmd对象
    void ReleaseCmd(__CmdBase * cmd){recvHelper_.ReleaseCmd()(cmd);}
    //将buf加入待发送缓冲区
    //buf会被清空
    bool AddOutBuf(__Buffer & buf, CSockAddr & udpClientAddr){
        return putBuf(buf, udpClientAddr, false);
    }
private:
    CSockSession(IFileDesc * fileDesc, const __RecvHelper & recvHelper);
    CSockSession(const CSockSession &);
    CSockSession & operator =(const CSockSession &);
    bool decodeCmd(__CmdBase *& cmd, size_t left);
    //增加/获取待发送的buf和addr
    bool getBuf(__Buffer & buf, CSockAddr & addr);
    bool putBuf(__Buffer & buf, CSockAddr & addr, bool front);
    //增加evetns
    void addEvents(__Events ev){ev_ |= ev;}
    //members
    IFileDesc * const fileDesc_;
    // recv
    const __RecvHelper & recvHelper_;
    __Buffer recvBuf_;  //operated only in AsyncIO
    size_t needSz_;     //operated only in AsyncIO
    // send
    __LockType sendLock_;
    __BufList outList_;
    __AddrList addrList_;
    // events
    __LockEvents ev_;
};

typedef CSockSession __SockSession;
typedef CSharedPtr<__SockSession> __SockPtr;
typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;

typedef CLockQueue<int> __FdQue;
typedef __FdQue::container_type __FdList;

NS_SERVER_END

#endif


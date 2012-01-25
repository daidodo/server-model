#ifndef DOZERG_SOCK_SESSION_H_20120122
#define DOZERG_SOCK_SESSION_H_20120122

#include <memory>
#include <string>
#include <vector>
#include <list>

#include <FileDesc.h>
#include <SharedPtr.h>
#include <Sockets.h>
#include <FdMap.h>
#include "Events.h"
#include "Command.h"

NS_SERVER_BEGIN

typedef std::list<__Buffer> __BufList;

enum EHandleDataRet
{
    RR_COMPLETE,
    RR_NEED_MORE,
    RR_ERROR
};
typedef std::pair<EHandleDataRet, size_t> __OnDataArriveRet;
typedef __OnDataArriveRet (*__OnDataArrive)(const __Buffer &);

struct CRecvHelper
{
    CRecvHelper()
        : onArrive_(0)
        , initSz_(0)
    {}
    bool IsValid() const{return onArrive_ && initSz_;}
    std::string ToString() const{return "";}
    //设置接收数据处理函数
    void OnDataArrive(__OnDataArrive onDataArrive){onArrive_ = onDataArrive;}
    __OnDataArrive OnDataArrive() const{return onArrive_;}
    //设置初始接收字节数
    void InitRecvSize(size_t sz){initSz_ = sz;}
    size_t InitRecvSize() const{return initSz_;}
private:
    __OnDataArrive onArrive_;
    size_t initSz_;
};

struct CSockSession
{
    typedef std::allocator<CSockSession> allocator_type;
    //functions
    static CSockSession * GetObject(const CRecvHelper & recvHelper){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession(recvHelper);
    }
    CSockSession(const CRecvHelper & recvHelper);
    ~CSockSession();
    int Fd() const{return fileDesc_->Fd();}
    bool IsValid() const{return fileDesc_ && fileDesc_->IsValid();}
    void Close(){fileDesc_->Close();}
    std::string ToString() const{return "";}
    //获取/设置事件标志
    __Events Events() const{return ev_;}
    void Events(__Events events){ev_ = events;}
    //接受新tcp客户端
    //client: 返回成功接受的client
    //return: true-正常; false-出错
    bool Accept(CSockSession *& client);
    //接收数据，decode cmd
    //cmd: 返回成功解码的cmd
    //udpClientAddr: 如果是udp连接，返回对方地址
    //return: true-正常; false-出错
    bool RecvCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
    //发送缓冲区的数据
    //return: true-正常; false-出错
    bool SendBuffer();
    //将缓冲区的数据写入文件
    //return: true-正常; false-出错
    bool WriteData();
    //处理cmd
    //udpClientAddr: 如果是udp连接，表示对方地址
    void Process(__CmdBase & cmd, const CSockAddr & udpClientAddr){}
private:
    bool recvTcpCmd(__CmdBase *& cmd);
    bool recvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
    bool decodeCmd(__CmdBase *& cmd);
    bool tcpSend();
    bool udpSend();
    //members
    IFileDesc * fileDesc_;
    // recv
    const CRecvHelper & recvHelper_;
    __Buffer recvBuf_;
    size_t needSz_;
    size_t stepIndex_;
    // send
    __BufList sendList_;
    // events
    __Events ev_;
};

typedef CSockSession __SockSession;
typedef CSharedPtr<__SockSession> __SockPtr;
typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;

NS_SERVER_END

#endif


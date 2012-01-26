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

NS_SERVER_BEGIN

typedef std::vector<char> __Buffer;
typedef std::list<__Buffer> __BufList;

enum ECheckDataRet
{
    RR_COMPLETE,
    RR_NEED_MORE,
    RR_ERROR
};

class CCmdBase;
typedef CCmdBase __CmdBase;

typedef std::pair<ECheckDataRet, size_t> __OnDataArriveRet;
typedef __OnDataArriveRet (*__OnDataArrive)(const char *, size_t);
typedef __CmdBase * (*__DecodeCmd)(const char *, size_t);
typedef void (*__ReleaseCmd)(__CmdBase * cmd);

struct CRecvHelper
{
    //functions
    CRecvHelper()
        : onArrive_(0)
        , decodeCmd_(0)
        , releaseCmd_(0)
        , initSz_(0)
    {}
    bool IsUdpValid() const{return decodeCmd_ && releaseCmd_;}
    bool IsTcpValid() const{return onArrive_ && initSz_ && IsUdpValid();}
    std::string ToString() const{return "";}
    //设置初始接收字节数
    void InitRecvSize(size_t sz){initSz_ = sz;}
    size_t InitRecvSize() const{return initSz_;}
    //设置接收数据处理函数
    void OnDataArrive(__OnDataArrive onDataArrive){onArrive_ = onDataArrive;}
    __OnDataArrive OnDataArrive() const{return onArrive_;}
    //设置解析命令处理函数
    void DecodeCmd(__DecodeCmd decodeCmd){decodeCmd_ = decodeCmd;}
    __DecodeCmd DecodeCmd() const{return decodeCmd_;}
    //设置释放命令处理函数
    void ReleaseCmd(__ReleaseCmd releaseCmd){releaseCmd_ = releaseCmd;}
    __ReleaseCmd ReleaseCmd() const{return releaseCmd_;}
private:
    //members
    __OnDataArrive onArrive_;
    __DecodeCmd decodeCmd_;
    __ReleaseCmd releaseCmd_;
    size_t initSz_;
};

class CSockSession
{
    typedef std::list<CSockAddr> __AddrList;
public:
    typedef CRecvHelper __RecvHelper;
    typedef std::allocator<CSockSession> allocator_type;
    //functions
    static CSockSession * GetObject(IFileDesc * fileDesc, const __RecvHelper & recvHelper){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession(fileDesc, recvHelper);
    }
    CSockSession(IFileDesc * fileDesc, const __RecvHelper & recvHelper);
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
    //释放cmd对象
    void ReleaseCmd(__CmdBase * cmd);
    //将buf加入待发送缓冲区
    //buf会被清空
    bool AddOutBuf(__Buffer & buf, CSockAddr & udpClientAddr);
    //发送缓冲区的数据
    //return: true-正常; false-出错
    bool SendBuffer();
    //将缓冲区的数据写入文件
    //return: true-正常; false-出错
    bool WriteData();
    //处理cmd
    //udpClientAddr: 如果是udp连接，表示对方地址
    void Process(__CmdBase & cmd, CSockAddr & udpClientAddr);
private:
    bool recvTcpCmd(__CmdBase *& cmd);
    bool recvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
    bool decodeCmd(__CmdBase *& cmd, size_t left);
    bool tcpSend();
    bool udpSend();
    //members
    IFileDesc * fileDesc_;
    // recv
    const __RecvHelper & recvHelper_;
    __Buffer recvBuf_;
    size_t needSz_;
    size_t stepIndex_;
    // send
    __BufList outList_;
    __AddrList addrList_;
    // events
    __Events ev_;
};

typedef CSockSession __SockSession;
typedef CSharedPtr<__SockSession> __SockPtr;
typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;

NS_SERVER_END

#endif


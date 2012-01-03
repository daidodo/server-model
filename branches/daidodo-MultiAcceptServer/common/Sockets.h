#ifndef DOZERG_SOCKTS_H_20080229
#define DOZERG_SOCKTS_H_20080229

/*
    对网络socket的简单包装
    隐藏了IPv4和IPv6协议相关性
        CSockAddr       IPv4或IPv6地址
        CSocket
        CTcpConnSocket  tcp客户端socket
        CListenSocket   tcp服务器端监听socket
        CUdpSocket
//*/

#include <errno.h>
#include <sys/socket.h>     //sockaddr
#include <vector>           //std::vector
#include <string>           //std::string
#include <Tools.h>          //Tools::ErrorMsg

NS_SERVER_BEGIN

class CSocket;
class CTcpConnSocket;
class CListenSocket;
class CUdpSocket;

class CSockAddr
{
    friend class CSocket;
    friend class CTcpConnSocket;
    friend class CListenSocket;
    friend class CUdpSocket;
    typedef struct sockaddr         __SA;
    typedef struct sockaddr_in      __SA4;
    typedef struct sockaddr_in6     __SA6;
    typedef struct sockaddr_storage __SS;
    typedef struct addrinfo         __AI;
    enum EAddrType{ADDR_SS,ADDR_SA4,ADDR_SA6};
    static int gai_errno;
    std::vector<char> sa_;
public:
    static std::string ErrMsg();
    CSockAddr(){}
    CSockAddr(const std::string & ip,const std::string & port){     //ip和port必须是地址和端口号，而不能是主机名和服务名
        SetAddr(ip,port);
    }
    CSockAddr(const std::string & ip,U16 port,bool hostByteOrder = true){
        SetAddr(ip,port,hostByteOrder);
    }
    std::string ToString() const;
    bool SetAddr(const std::string & ip,const std::string & port);  //ip和port必须是地址和端口号，而不能是主机名和服务名
    bool SetAddr(const std::string & ip,U16 port,bool hostByteOrder = true){
        return SetAddr(ip,"") && SetPort(port,hostByteOrder);
    }
	bool SetAddr(const std::string & eth);		//获取接口地址
    void SetIP(U32 ip4,bool hostByteOrder = true);
    void SetIP(const void * ip6);
    bool SetPort(U16 port,bool hostByteOrder = true);
    U16 GetPort(bool hostByteOrder = true) const;
    U32 GetIPv4(bool hostByteOrder = true) const;
    bool IsValid() const;
private:
    socklen_t sockLen() const{return socklen_t(sa_.size());}
    socklen_t format(EAddrType at);
    int familyType() const{return (sa_.empty() ? AF_UNSPEC : SA()->sa_family);}
    __SA * SA(){return sa_.empty() ? 0 : (__SA *)(&sa_[0]);}
    __SA4 * SA4(){return sa_.empty() ? 0 : (__SA4 *)(&sa_[0]);}
    __SA6 * SA6(){return sa_.empty() ? 0 : (__SA6 *)(&sa_[0]);}
    const __SA * SA() const{return sa_.empty() ? 0 : (const __SA *)(&sa_[0]);}
    const __SA4 * SA4() const{return sa_.empty() ? 0 : (const __SA4 *)(&sa_[0]);}
    const __SA6 * SA6() const{return sa_.empty() ? 0 : (const __SA6 *)(&sa_[0]);}
};

class CSocket
{
    CSocket(const CSocket &);
    CSocket & operator =(const CSocket &);
public:
    enum ESockType{ //type of connection
        TCP,        //SOCK_STREAM + IPPROTO_TCP
        UDP         //SOCK_DGRAM + IPPROTO_UDP
    };
    static const int INVALID_FD = -1;
    static std::string ErrMsg(){return Tools::ErrorMsg(errno);}
    CSocket();
    virtual ~CSocket();
    int Fd() const{return fd_;}
    bool IsValid() const{return fd_ != INVALID_FD;}
    bool SetLinger(bool on = true,int timeout = 0);
    bool SetBlock(bool on = true);
    bool SetReuse(bool on = true);
    bool SetSendTimeout(U32 timeMs);
    bool SetRecvTimeout(U32 timeMs);
    bool SetSendSize(size_t sz);
    bool SetRecvSize(size_t sz);
    size_t GetSendSize() const;
    size_t GetRecvSize() const;
    void Close();
    ssize_t RecvData(char * buf,size_t sz,bool block = false);
    ssize_t RecvData(std::vector<char> & buf,size_t sz,bool block = false);
    ssize_t RecvData(std::string & buf,size_t sz,bool block = false);
    //在指定的时间timeoutMs内发送数据buf,必须是非阻塞模式
    //返回是否发送完成
    bool SendData(const char * buf,size_t sz,U32 timeoutMs);
    bool SendData(const std::vector<char> & buf,U32 timeoutMs){
        return SendData(&buf[0],buf.size(),timeoutMs);
    }
    bool SendData(const std::string & buf,U32 timeoutMs){
        return SendData(&buf[0],buf.length(),timeoutMs);
    }
    //发送数据buf,直接调用send
    //返回：+n,实际发送的字节数；0,需要重试；-n，出错
    ssize_t SendData(const std::vector<char> & buf);
protected:
    bool getSock(int family,ESockType socktype);
    bool bindAddr(const CSockAddr & addr);
    bool connectAddr(const CSockAddr & addr);
    int fd_;
};

class CTcpConnSocket : public CSocket
{
    friend class CListenSocket;
    CSockAddr peerAddr_;
public:
    std::string ToString() const;
    const CSockAddr & PeerAddr() const{return peerAddr_;}
    bool Connect(const CSockAddr & addr);
    bool Reconnect();   //close and connect again
};

class CListenSocket : public CSocket
{
    static const int DEFAULT_LISTEN_QUEUE = 1024;
public:
    //Accept()的返回值
    static const int RET_SUCC = 0;
    static const int RET_EAGAIN = 1;
    static const int RET_ERROR = 2;
    std::string ToString() const;
    bool Listen(const CSockAddr & addr,bool block = true,int queueSz = DEFAULT_LISTEN_QUEUE);
    //return: RET_xxx
    int Accept(CTcpConnSocket & sock) const;
private:
    CSockAddr hostAddr_;
};

class CUdpSocket : public CSocket
{
    CSockAddr hostAddr_,peerAddr_;
public:
    std::string ToString() const;
    bool Socket(const CSockAddr & addr);
    bool Bind(const CSockAddr & addr);      //指定本机地址hostAddr_
    bool Connect(const CSockAddr & addr);   //指定对方地址peerAddr_,可多次调用指定不同的对方地址
    const CSockAddr & HostAddr() const{return hostAddr_;}
    const CSockAddr & PeerAddr() const{return peerAddr_;}
    //from返回谁发送了数据,如果不需要,请使用CSocket::RecvData
    ssize_t RecvData(CSockAddr & from,std::vector<char> & buf,size_t sz,bool block = false);
    bool SendData(const CSockAddr & to,const std::vector<char> & buf,U32 timeoutMs);
    using CSocket::RecvData;
    using CSocket::SendData;
private:
    bool ensureSock(const CSockAddr & addr);
};

NS_SERVER_END

#endif


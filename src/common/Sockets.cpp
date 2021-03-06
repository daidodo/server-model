#include <sys/ioctl.h>      //ioctl,SIOCGIFADDR
#include <arpa/inet.h>      //inet_ntop,sockaddr_in
#include <net/if.h>         //ifreq
#include <netdb.h>          //getaddrinfo,freeaddrinfo,gai_strerror
#include <fcntl.h>          //fcntl
#include <unistd.h>
#include <sstream>
#include <cassert>
#include "Sockets.h"

NS_SERVER_BEGIN

//class CSockAddr
int CSockAddr::gai_errno = 0;

std::string CSockAddr::ErrMsg()
{
    return std::string(", ") + gai_strerror(gai_errno);
}

std::string CSockAddr::ToString() const
{
    const char * UNKNOWN = "unknown";
    if(sa_.size() < sizeof(__SA4))
        return UNKNOWN;
    char str[128];  //Unix domain is largest
    std::ostringstream oss;
	switch(SA()->sa_family){
        case AF_INET:{
            if(!inet_ntop(AF_INET,&(SA4()->sin_addr),str,sizeof(str)))
                return UNKNOWN;
            oss<<str<<":"<<ntohs(SA4()->sin_port);
            break;}
#ifdef AF_INET6
        case AF_INET6:{
            if(!inet_ntop(AF_INET6,&(SA6()->sin6_addr),str,sizeof(str)))
                return UNKNOWN;
            oss<<"["<<str<<"]:"<<ntohs(SA6()->sin6_port);
            break;}
#endif
        default:
            return UNKNOWN;
    }
    return oss.str();
}

bool CSockAddr::SetAddr(const std::string & ip,const std::string & port)
{
    typedef struct addrinfo __AI;
    __AI hints;
    memset(&hints,0,sizeof hints);
    //hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
#ifdef AI_NUMERICSERV
    //hints.ai_flags |= AI_NUMERICSERV;
#endif
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    __AI * ret;
    int n = getaddrinfo(ip.c_str(),port.c_str(),&hints,&ret);
    if(n != 0){
        gai_errno = n;
        return false;
    }
    char * p = (char *)(ret->ai_addr);
    sa_.assign(p,p + ret->ai_addrlen);
    freeaddrinfo(ret);
    return true;
}

bool CSockAddr::SetAddr(const std::string & eth)
{
    if(eth.length() >= IFNAMSIZ)
        return false;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    memcpy(ifr.ifr_name, eth.c_str(), eth.length());
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        return false;
    if(ioctl(fd, SIOCGIFADDR, &ifr) < 0)
        return false;
    sa_.resize(sizeof(struct sockaddr));
    memcpy(&sa_[0], &ifr.ifr_addr, sizeof(struct sockaddr));
    close(fd);
    return true;
}

void CSockAddr::SetIP(U32 ip4,bool hostByteOrder)
{
    if(sa_.size() != sizeof(__SA4))
        format(ADDR_SA4);
    SA4()->sin_addr.s_addr = hostByteOrder ? htonl(ip4) : ip4;
}

void CSockAddr::SetIP(const void * ip6)
{
    if(sa_.size() != sizeof(__SA6))
        format(ADDR_SA6);
    memcpy(&(SA6()->sin6_addr),ip6,sizeof(struct in6_addr));
}

bool CSockAddr::SetPort(U16 port,bool hostByteOrder)
{
    if(hostByteOrder)
        port = htons(port);
    if(sa_.size() != sizeof(__SA4)){
        SA4()->sin_port = port;
    }else if(sa_.size() != sizeof(__SA6)){
        SA6()->sin6_port = port;
    }else
        return false;
    return true;
}

U16 CSockAddr::GetPort(bool hostByteOrder) const
{
    U16 ret = 0;
    if(sa_.size() == sizeof(__SA4))
        ret = SA4()->sin_port;
    else if(sa_.size() == sizeof(__SA6))
        ret = SA6()->sin6_port;
    return hostByteOrder ? ntohs(ret) : ret;
}

U32 CSockAddr::GetIPv4(bool hostByteOrder) const
{
    U32 ret = 0;
    if(sa_.size() == sizeof(__SA4))
        ret = SA4()->sin_addr.s_addr;
    return hostByteOrder ? ntohl(ret) : ret;
}

socklen_t CSockAddr::format(EAddrType at)
{
    typedef struct sockaddr_storage __SS;
    U16 port = GetPort(false);
    switch(at){
        case ADDR_SA4:   //IPv4
            sa_.assign(sizeof(__SA4),0);
            SA()->sa_family = AF_INET;
            SA4()->sin_port = port;
            break;
        case ADDR_SA6:   //IPv6
            sa_.assign(sizeof(__SA6),0);
            SA()->sa_family = AF_INET6;
            SA6()->sin6_port = port;
            break;
        default:        //max IP storage
            sa_.assign(sizeof(__SS),0);
    }
    return sockLen();
}
bool CSockAddr::IsValid() const
{
    return ((sa_.size() == sizeof(__SA4) && SA()->sa_family == AF_INET)) ||
        ((sa_.size() == sizeof(__SA6) && SA()->sa_family == AF_INET6));
}

//class CSocket
bool CSocket::SetLinger(bool on,int timeout)
{
    if(!IsValid())
        return false;
    struct linger ling;
    ling.l_onoff = on ? 1 : 0;
    ling.l_linger = timeout;
    if(setsockopt(fd_,SOL_SOCKET,SO_LINGER,&ling,sizeof ling) < 0)
        return false;
    return true;
}

bool CSocket::SetReuse(bool on)
{
    if(!IsValid())
        return false;
    int flag = on ? 1 : 0;
    return setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof flag) == 0;
}

bool CSocket::SetSendTimeout(U32 timeMs)
{
    struct timeval tv;
    tv.tv_sec = timeMs / 1000;
    tv.tv_usec = timeMs % 1000;
    return setsockopt(fd_,SOL_SOCKET,SO_RCVTIMEO,(const char *)&tv,sizeof tv) == 0;
}

bool CSocket::SetRecvTimeout(U32 timeMs)
{
    struct timeval tv;
    tv.tv_sec = timeMs / 1000;
    tv.tv_usec = timeMs % 1000;
    return setsockopt(fd_,SOL_SOCKET,SO_SNDTIMEO,(const char *)&tv,sizeof tv) == 0;
}

bool CSocket::SetSendSize(size_t sz)
{
    if(!IsValid())
        return false;
    return setsockopt(fd_,SOL_SOCKET,SO_SNDBUF,&sz,sizeof sz) == 0;
}

bool CSocket::SetRecvSize(size_t sz)
{
    if(!IsValid())
        return false;
    return setsockopt(fd_,SOL_SOCKET,SO_RCVBUF,&sz,sizeof sz) == 0;
}

size_t CSocket::GetSendSize() const
{
    if(!IsValid())
        return false;
    size_t ret = 0;
    socklen_t len = sizeof ret;
    if(getsockopt(fd_,SOL_SOCKET,SO_SNDBUF,&ret,&len) < 0)
        return 0;
    return ret;
}

size_t CSocket::GetRecvSize() const
{
    if(!IsValid())
        return false;
    size_t ret = 0;
    socklen_t len = sizeof ret;
    if(getsockopt(fd_,SOL_SOCKET,SO_RCVBUF,&ret,&len) < 0)
        return 0;
    return ret;
}

ssize_t CSocket::RecvData(char * buf,size_t sz,bool block)
{
    assert(buf);
    if(!CSocket::IsValid())
        return -1;
    ssize_t ret = recv(CSocket::Fd(),buf,sz,(block ? MSG_WAITALL : 0));
    return ret;
}

ssize_t CSocket::RecvData(std::string & buf,size_t sz,bool block)
{
    if(!CSocket::IsValid())
        return -1;
    size_t oldsz = buf.size();
    buf.resize(oldsz + sz);
    ssize_t ret = recv(CSocket::Fd(),&buf[oldsz],sz,(block ? MSG_WAITALL : 0));
    if(ret <= 0)
        buf.resize(oldsz);
    else if(size_t(ret) < sz)
        buf.resize(oldsz + ret);
    return ret;
}

bool CSocket::SendData(const char * buf,size_t size,U32 timeoutMs)
{
    assert(buf);
    if(!CSocket::IsValid())
        return false;
    if(!timeoutMs)
        return send(CSocket::Fd(),&buf[0],size,MSG_NOSIGNAL) == int(size);
    for(size_t sz = 0,total = size;sz < total;){
        int n = send(CSocket::Fd(),&buf[sz],total - sz,MSG_NOSIGNAL);
        if(n >= 0)
            sz += n;
        else if(errno == EAGAIN || errno == EWOULDBLOCK){
            struct pollfd poll_fd;
            memset(&poll_fd, 0,sizeof poll_fd);
            poll_fd.fd = CSocket::Fd();
            poll_fd.events = POLLOUT | POLLWRNORM;
            poll_fd.revents = 0;
            if(poll(&poll_fd,1,timeoutMs) != 1)
                return false;
            timeoutMs >>= 1;
        }else
            return false;
    }
    return true;
}

bool CSocket::getSock(int family,ESockType socktype)
{
    if(IsValid())
        return false;
    switch(socktype){
        case TCP:     //TCP
            fd_ = socket(family,SOCK_STREAM,IPPROTO_TCP);
            break;
        case UDP:     //UDP
            fd_ = socket(family,SOCK_DGRAM,IPPROTO_UDP);
            break;
        default:
            return false;
    }
    return IsValid();
}

bool CSocket::bindAddr(const CSockAddr & addr)
{
    if(IsValid() && addr.IsValid())
        return bind(Fd(),addr.SA(),addr.sockLen()) == 0;
    return false;
}

bool CSocket::connectAddr(const CSockAddr & addr)
{
    if(IsValid() && addr.IsValid())
        return connect(fd_,addr.SA(),addr.sockLen()) == 0;
    return false;
}

//class CTcpConnSocket
std::string CTcpConnSocket::ToString() const
{
    std::ostringstream oss;
    oss<<"{(CTcpConnSocket)base="<<CSocket::ToString()
        <<", peerAddr_="<<peerAddr_.ToString()
        <<"}";
    return oss.str();
}

bool CTcpConnSocket::Connect(const CSockAddr & addr)
{
    if(!IsValid() &&
        addr.IsValid() &&
        CSocket::getSock(addr.familyType(),CSocket::TCP) &&
        CSocket::connectAddr(addr))
    {
        peerAddr_ = addr;
        return true;
    }
    CSocket::Close();
    peerAddr_ = addr;
    return false;
}

bool CTcpConnSocket::Reconnect()
{
    if(!peerAddr_.IsValid())
        return false;
    if(IsValid())
        CSocket::Close();
    if(CSocket::getSock(peerAddr_.familyType(),CSocket::TCP) &&
        CSocket::connectAddr(peerAddr_))
        return true;
    return false;
}

//class CListenSocket
std::string CListenSocket::ToString() const
{
    std::ostringstream oss;
    oss<<"{(CListenSocket)base="<<CSocket::ToString()
        <<", hostAddr_="<<hostAddr_.ToString()
        <<"}";
    return oss.str();
}

bool CListenSocket::Listen(const CSockAddr & addr,bool block,int queueSz)
{
    if(queueSz <= 0)
        queueSz = DEFAULT_LISTEN_QUEUE;
    if(!CSocket::IsValid() &&
        addr.IsValid() &&
        CSocket::getSock(addr.familyType(),CSocket::TCP) &&
        CSocket::SetReuse() &&
        CSocket::SetBlock(block) &&
        CSocket::bindAddr(addr) &&
        listen(CSocket::Fd(),queueSz) == 0)
    {
        hostAddr_ = addr;
        return true;
    }
    CSocket::Close();
    return false;
}

int CListenSocket::Accept(CTcpConnSocket & sock) const
{
    if(!CSocket::IsValid())
        return RET_ERROR;
    if(sock.IsValid())
        return RET_SUCC;
    socklen_t len = sock.peerAddr_.format(CSockAddr::ADDR_SS);
    sock.fd_ = accept(CSocket::Fd(),sock.peerAddr_.SA(),&len);
    if(sock.IsValid()){
        sock.peerAddr_.sa_.resize(len);
        return RET_SUCC;
    }else if(errno == EAGAIN || errno == EWOULDBLOCK)
        return RET_EAGAIN;
    return RET_ERROR;
}

//class CUdpSocket
std::string CUdpSocket::ToString() const
{
    std::ostringstream oss;
    oss<<"{(CUdpSocket)base="<<CSocket::ToString()
        <<",hostAddr_="<<hostAddr_.ToString()
        <<",peerAddr_="<<peerAddr_.ToString()
        <<"}";
    return oss.str();
}

bool CUdpSocket::Socket(const CSockAddr & addr)
{
    if(!CSocket::IsValid() && addr.IsValid())
        return CSocket::getSock(addr.familyType(),CSocket::UDP);
    return false;
}

bool CUdpSocket::Bind(const CSockAddr & addr)
{
    if(ensureSock(addr) && CSocket::bindAddr(addr)){
        hostAddr_ = addr;
        return true;
    }
    return false;
}

bool CUdpSocket::Connect(const CSockAddr & addr)
{
    if(ensureSock(addr) && CSocket::connectAddr(addr)){
        peerAddr_ = addr;
        return true;
    }
    return false;
}

bool CUdpSocket::ensureSock(const CSockAddr & addr)
{
    if(CSocket::IsValid())
        return true;
    return Socket(addr);
}

NS_SERVER_END


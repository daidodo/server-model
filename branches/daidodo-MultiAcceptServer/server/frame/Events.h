#ifndef DOZERG_EVENTS_H_20120122
#define DOZERG_EVENTS_H_20120122

#include <string>

#include <Tools.h>

NS_SERVER_BEGIN

//event flags
typedef U32 __Events;

const __Events EVENT_CLOSE = 1 << 0;
const __Events EVENT_IN = 1 << 1;
const __Events EVENT_OUT = 1 << 2;
const __Events EVENT_ACCEPT = 1 << 3;
const __Events EVENT_TCP_RECV = 1 << 4;
const __Events EVENT_UDP_RECV = 1 << 5;
const __Events EVENT_TCP_SEND = 1 << 6;
const __Events EVENT_UDP_SEND = 1 << 7;
const __Events EVENT_READ = 1 << 8;
const __Events EVENT_WRITE = 1 << 9;

namespace Events{
    inline bool NeedClose(__Events ev){return (ev & EVENT_CLOSE);}
    inline bool CanInput(__Events ev){return (ev & EVENT_IN);}
    inline bool CanOutput(__Events ev){return (ev & EVENT_OUT);}
    inline bool CanAccept(__Events ev){return (ev & EVENT_ACCEPT);}
    inline bool CanTcpRecv(__Events ev){return (ev & EVENT_TCP_RECV);}
    inline bool CanUdpRecv(__Events ev){return (ev & EVENT_UDP_RECV);}
    inline bool CanTcpSend(__Events ev){return (ev & EVENT_TCP_SEND);}
    inline bool CanUdpSend(__Events ev){return (ev & EVENT_UDP_SEND);}
    inline bool CanRecv(__Events ev){
        return CanTcpRecv(ev)
            || CanUdpRecv(ev);
    }
    inline bool CanSend(__Events ev){
        return CanTcpSend(ev)
            || CanUdpSend(ev);
    }
    inline bool CanRead(__Events ev){return (ev & EVENT_READ);}
    inline bool CanWrite(__Events ev){return (ev & EVENT_WRITE);}
    inline bool NeedInput(__Events ev){
        return CanRecv(ev)
            || CanAccept(ev)
            || CanRead(ev);
    }
    inline bool NeedOutput(__Events ev){
        return CanSend(ev)
            || CanWrite(ev);
    }
    inline std::string ToString(__Events ev){
        const char * const BIT_NAME[] = {
            "CLOSE",
            "IN",
            "OUT",
            "ACCEPT",
            "TCP_RECV",
            "UDP_RECV",
            "TCP_SEND",
            "UDP_SEND",
            "READ",
            "WRITE",
        };
        return Tools::ToStringBits(ev, BIT_NAME, Tools::ArraySize(BIT_NAME));
    }
}//namespace Events

struct CFdEvent
{
    //functions
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

typedef std::string __Buffer;

NS_SERVER_END

#endif


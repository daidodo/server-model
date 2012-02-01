#ifndef DOZERG_CMD_SESSION_H_20120122
#define DOZERG_CMD_SESSION_H_20120122

#include <memory>

#include <Sockets.h>

NS_SERVER_BEGIN

class CRecvHelper;

struct CCmdSession
{
    typedef std::allocator<CCmdSession> allocator_type;
    //functions
    static CCmdSession * GetObject(int fd, U32 fingerPrint, CCmdBase * cmd, const CRecvHelper & recvHelper, CSockAddr & udpClientAddr);
    ~CCmdSession();
    int Fd() const{return fd_;}
    U32 FingerPrint() const{return finger_;}
    CCmdBase * CmdBase() const{return cmd_;}
    CSockAddr & UdpClientAddr(){return udpClientAddr_;}
    std::string ToString() const;
private:
    CCmdSession(int fd, U32 fingerPrint, CCmdBase * cmd, const CRecvHelper & recvHelper, CSockAddr & udpClientAddr);
	CCmdSession(const CCmdSession &);
	CCmdSession operator =(const CCmdSession &);
    //members
    const int fd_;
    const U32 finger_;
    CCmdBase * const cmd_;
    const CRecvHelper & recvHelper_;
    CSockAddr udpClientAddr_;
};

NS_SERVER_END

#endif


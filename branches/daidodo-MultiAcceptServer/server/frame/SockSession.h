#ifndef DOZERG_SOCK_SESSION_H_20120122
#define DOZERG_SOCK_SESSION_H_20120122

#include <memory>
#include <string>
#include <list>

#include <FileDesc.h>
#include <SharedPtr.h>
#include <Sockets.h>
#include <LockInt.h>
#include <LockQueue.h>
#include <FdMap.h>
#include "Events.h"

NS_SERVER_BEGIN

class CCmdBase;
class CRecvHelper;

class CSockSession
{
    typedef std::string __Buffer;
    typedef std::list<__Buffer> __BufList;
    typedef CMutex __LockType;
    typedef CGuard<__LockType> __Guard;
    typedef CLockInt<__Events> __LockEvents;
    typedef std::list<CSockAddr> __AddrList;
public:
    typedef std::allocator<CSockSession> allocator_type;
    //functions
    static CSockSession * GetObject(IFileDesc * fileDesc, const CRecvHelper & recvHelper){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession(fileDesc, recvHelper);
    }
    ~CSockSession();
    int Fd() const{return fileDesc_->Fd();}
    EFileDescType FileType() const{return fileDesc_->Type();}
    bool IsValid() const{return fileDesc_ && fileDesc_->IsValid();}
    void Close(){fileDesc_->Close();}
    U32 FingerPrint() const{return finger_;}
    std::string ToString() const;
    //��ȡrecv helper
    const CRecvHelper & RecvHelper() const{return recvHelper_;}
    //��ȡ/�����¼���־
    __Events Events() const{return ev_;}
    void Events(__Events events){ev_ = events;}
    //������tcp�ͻ���
    //client: ���سɹ����ܵ�client
    //events: ����client����һ���¼�(EVENT_OUT, EVENT_IN)
    //return: true-����; false-����
    bool Accept(CSockSession *& client, __Events & events);
    //�������ݣ�decode cmd
    //cmd: ���سɹ������cmd
    //udpClientAddr: �����udp���ӣ����ضԷ���ַ
    bool RecvTcpCmd(CCmdBase *& cmd);
    bool RecvUdpCmd(CCmdBase *& cmd, CSockAddr & udpClientAddr);
    //���ͻ�����������
    bool SendTcpData();
    bool SendUdpData();
    //��������������д���ļ�
    bool WriteData();
    //����cmd
    //udpClientAddr: �����udp���ӣ���ʾ�Է���ַ
    //return: EVENT_OUT-��Ҫoutput; EVENT_IN-��Ҫinput; EVENT_CLOSE-��Ҫclose
    __Events Process(CCmdBase & cmd, CSockAddr & udpClientAddr);
    //��buf��������ͻ�����
    //buf�ᱻ���
    bool AddOutBuf(__Buffer & buf, CSockAddr & udpClientAddr){
        return putBuf(buf, udpClientAddr, false);
    }
private:
    CSockSession(IFileDesc * fileDesc, const CRecvHelper & recvHelper);
    CSockSession(const CSockSession &);
    CSockSession & operator =(const CSockSession &);
    bool decodeCmd(CCmdBase *& cmd, size_t left);
    //����/��ȡ�����͵�buf��addr
    bool getBuf(__Buffer & buf, CSockAddr & addr);
    bool putBuf(__Buffer & buf, CSockAddr & addr, bool front);
    //����evetns
    void addEvents(__Events ev){ev_ |= ev;}
    //members
    IFileDesc * const fileDesc_;
    const U32 finger_;
    // recv
    const CRecvHelper & recvHelper_;
    __Buffer recvBuf_;  //operated only in AsyncIO
    size_t needSz_;     //operated only in AsyncIO
    // send
    __LockType sendLock_;
    __BufList outList_;
    __AddrList addrList_;
    // events
    __LockEvents ev_;
};

NS_SERVER_END

#endif


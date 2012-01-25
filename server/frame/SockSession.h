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
    //���ý������ݴ�����
    void OnDataArrive(__OnDataArrive onDataArrive){onArrive_ = onDataArrive;}
    __OnDataArrive OnDataArrive() const{return onArrive_;}
    //���ó�ʼ�����ֽ���
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
    //��ȡ/�����¼���־
    __Events Events() const{return ev_;}
    void Events(__Events events){ev_ = events;}
    //������tcp�ͻ���
    //client: ���سɹ����ܵ�client
    //return: true-����; false-����
    bool Accept(CSockSession *& client);
    //�������ݣ�decode cmd
    //cmd: ���سɹ������cmd
    //udpClientAddr: �����udp���ӣ����ضԷ���ַ
    //return: true-����; false-����
    bool RecvCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
    //���ͻ�����������
    //return: true-����; false-����
    bool SendBuffer();
    //��������������д���ļ�
    //return: true-����; false-����
    bool WriteData();
    //����cmd
    //udpClientAddr: �����udp���ӣ���ʾ�Է���ַ
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


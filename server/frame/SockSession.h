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
typedef __OnDataArriveRet (*__OnDataArrive)(const __Buffer &);
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
    //���ó�ʼ�����ֽ���
    void InitRecvSize(size_t sz){initSz_ = sz;}
    size_t InitRecvSize() const{return initSz_;}
    //���ý������ݴ�����
    void OnDataArrive(__OnDataArrive onDataArrive){onArrive_ = onDataArrive;}
    __OnDataArrive OnDataArrive() const{return onArrive_;}
    //���ý����������
    void DecodeCmd(__DecodeCmd decodeCmd){decodeCmd_ = decodeCmd;}
    __DecodeCmd DecodeCmd() const{return decodeCmd_;}
    //�����ͷ��������
    void ReleaseCmd(__ReleaseCmd releaseCmd){releaseCmd_ = releaseCmd;}
    __ReleaseCmd ReleaseCmd() const{return releaseCmd_;}
private:
    //members
    __OnDataArrive onArrive_;
    __DecodeCmd decodeCmd_;
    __ReleaseCmd releaseCmd_;
    size_t initSz_;
};

struct CSockSession
{
    typedef std::allocator<CSockSession> allocator_type;
    typedef CRecvHelper __RecvHelper;
    //functions
    static CSockSession * GetObject(const __RecvHelper & recvHelper){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession(recvHelper);
    }
    CSockSession(const __RecvHelper & recvHelper);
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
    //�ͷ�cmd����
    void ReleaseCmd(__CmdBase * cmd);
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
    __BufList sendList_;
    // events
    __Events ev_;
};

typedef CSockSession __SockSession;
typedef CSharedPtr<__SockSession> __SockPtr;
typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;

NS_SERVER_END

#endif


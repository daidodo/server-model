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
#include "CmdBase.h"

NS_SERVER_BEGIN

typedef std::vector<char> __Buffer;
typedef std::list<__Buffer> __BufList;
typedef std::pair<int, size_t> (*__OnRecv)(const __Buffer &);

struct CRecvHelper
{
    CRecvHelper()
        : initSz_(0)
    {}
    size_t InitSize() const{return initSz_;}
    size_t StepSize() const{return recvSteps_.size();}
    __OnRecv Step(size_t index) const{return recvSteps_[index];}
    //���ó�ʼ�����ֽ���
    void SetInitRecvSize(size_t sz){initSz_ = sz;}
    //���ý������ݴ�����
    void AddRecvStep(__OnRecv onRecv){recvSteps_.push_back(onRecv);}
private:
    std::vector<__OnRecv> recvSteps_;
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
    //����cmd
    //udpClientAddr: �����udp���ӣ���ʾ�Է���ַ
    void Process(__CmdBase & cmd, const CSockAddr & udpClientAddr){}
    //���ͻ�����������
    //return: true-����; false-����
    bool SendBuffer(){return true;}
    //��������������д���ļ�
    //return: true-����; false-����
    bool WriteData(){return true;}
private:
    bool recvTcpCmd(__CmdBase *& cmd);
    bool recvUdpCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr);
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


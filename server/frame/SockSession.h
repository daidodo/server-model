#ifndef DOZERG_SOCK_SESSION_H_20120122
#define DOZERG_SOCK_SESSION_H_20120122

#include <memory>
#include <string>
#include <vector>
#include <list>

#include <FileDesc.h>
#include <SharedPtr.h>
#include "Events.h"
#include "CmdBase.h"

NS_SERVER_BEGIN

typedef std::vector<char> __Buffer;
typedef std::list<__Buffer> __BufList;
typedef std::pair<int, size_t> (*__OnRecv)(const __Buffer &);

struct CSockSession
{
    typedef std::allocator<CSockSession> allocator_type;
    //functions
    static CSockSession * GetObject(){
        CSockSession * ret = allocator_type().allocate(1);
        return new (ret) CSockSession;
    }
    CSockSession()
        : fileDesc_(0)
        , needSz_(0)
        , initSz_(0)
        , ev_(0)
    {}
    int Fd() const{return fileDesc_->Fd();}
    bool IsValid() const{return fileDesc_->IsValid();}
    void Close(){fileDesc_->Close();}
    std::string ToString() const{return "";}
    //��ȡ/�����¼���־
    __Events Events() const{return ev_;}
    void Events(__Events events){ev_ = events;}
    //���ó�ʼ�����ֽ���
    void SetInitRecvSize(size_t sz){initSz_ = sz;}
    //���ý������ݴ�����
    void AddRecvStep(__OnRecv onRecv){recvSteps_.push_back(onRecv);}
    //�������ݣ�decode cmd
    //udpClientAddr: �����udp���ӣ����ضԷ���ַ
    //return: true-����; false-����
    bool RecvCmd(__CmdBase *& cmd, CSockAddr & udpClientAddr){return true;}
    //������tcp�ͻ���
    CSockSession * Accept(){
        //sock->SetLinger();
        //sock->SetBlock(false);
        return 0;
    }
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
    //members
    IFileDesc * fileDesc_;
    // recv
    std::vector<__OnRecv> recvSteps_;
    __Buffer recvBuf_;
    size_t needSz_;
    size_t initSz_;
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


#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

#include <vector>
#include <string>

#include <LockQueue.h>
#include <SharedPtr.h>
#include <FdMap.h>
#include <Sockets.h>

#include "CmdSession.h"

NS_SERVER_BEGIN

/*
typedef std::vector<char> __Buffer;
struct CCmdSock
{
    typedef std::allocator<CCmdSock> allocator_type;
    //constants
    static const int RET_IO_ERROR = -2;     //io����
    static const int RET_IO_RETRY = -1;     //io��Ҫ����
    static const int RET_IO_COMPLETE = 0;   //io�������
    static const int RET_CMD_ERROR = -1;    //��������
    static const int RET_CMD_SUCC = 0;      //��������
public:
    //functions
    static CCmdSock * GetObject(){
        CCmdSock * ret = allocator_type().allocate(1);
        return new (ret) CCmdSock;
    }
    CCmdSock()
        : fileDesc_(0)
        , ev_(0)
    {}
    int Fd() const{return fileDesc_->Fd();}
    void Close(){fileDesc_->Close();}
    //��ȡ/�����¼���־
    __Events Events() const{return ev_;}
    void Events(__Events ev){ev_ = ev;}
    //�������ݣ�decode cmd
    //return: �Ƿ����
    //���������ͨ��cmd����
    bool RecvTcpCmd(__CmdBase *& cmd);
    bool RecvUdpCmd(__CmdBase *& cmd, CCmdSock & sock){return true;}
    //������tcp�ͻ���
    CCmdSock * Accept(){
        //CSocket * sock = 0;
        //sock->SetLinger();
        //sock->SetBlock(false);
        return 0;
    }
    //���ͻ��������
    //return: �Ƿ����
    bool SendBuffer(){return true;}
    //�����������д���ļ�
    //return: �Ƿ����
    bool WriteData(){return true;}
    //��buf���뷢�ͻ���
    void AddSendBuf(__Buffer & buf){}
    //����cmd
    void Process(__CmdBase & cmd){}
    std::string ToString() const{return "";}
private:
    //members
    IFileDesc * fileDesc_;
    __Events ev_;
};
//*/

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef __FdEventQue::container_type __FdEventList;
typedef CLockQueue<int> __FdQue;
typedef __FdQue::container_type __FdList;

typedef std::vector<__SockPtr> __SockPtrList;
typedef std::vector<int> __FdArray;

NS_SERVER_END

#endif


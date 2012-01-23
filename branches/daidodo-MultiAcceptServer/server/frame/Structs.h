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
    static const int RET_IO_ERROR = -2;     //io出错
    static const int RET_IO_RETRY = -1;     //io需要重试
    static const int RET_IO_COMPLETE = 0;   //io操作完成
    static const int RET_CMD_ERROR = -1;    //命令处理出错
    static const int RET_CMD_SUCC = 0;      //命令处理完成
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
    //获取/设置事件标志
    __Events Events() const{return ev_;}
    void Events(__Events ev){ev_ = ev;}
    //接收数据，decode cmd
    //return: 是否出错
    //解出的命令通过cmd返回
    bool RecvTcpCmd(__CmdBase *& cmd);
    bool RecvUdpCmd(__CmdBase *& cmd, CCmdSock & sock){return true;}
    //接受新tcp客户端
    CCmdSock * Accept(){
        //CSocket * sock = 0;
        //sock->SetLinger();
        //sock->SetBlock(false);
        return 0;
    }
    //发送缓冲的数据
    //return: 是否出错
    bool SendBuffer(){return true;}
    //将缓冲的数据写入文件
    //return: 是否出错
    bool WriteData(){return true;}
    //将buf加入发送缓冲
    void AddSendBuf(__Buffer & buf){}
    //处理cmd
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


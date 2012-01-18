#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

#include <vector>

#include <LockQueue.h>
#include <SharedPtr.h>
#include <FdMap.h>
#include <Sockets.h>

NS_SERVER_BEGIN

struct CFdEvent
{
    static const int EVENT_READ = 1;
    static const int EVENT_WRITE = 2;
    static const int EVENT_CLOSE = 4;
    static const int EVENT_ADD = 8;
    static const int EVENT_READ_ADD = EVENT_READ | EVENT_ADD;
    static const int EVENT_WRITE_ADD = EVENT_WRITE | EVENT_ADD;
    static const int & ExtractFd(const CFdEvent & fe){return fe.fd_;}
    CFdEvent(int f, int e)
        : fd_(f)
        , event_(e)
    {}
    int Fd() const{return fd_;}
    bool Readable() const{return (event_ & EVENT_READ);}
    bool Writable() const{return (event_ & EVENT_WRITE);}
    bool Closable() const{return (event_ & EVENT_CLOSE);}
    bool AddFlags() const{return (event_ & EVENT_ADD);}
private:
    //members:
    int fd_;
    int event_;
};

struct CCmdBase
{
    typedef std::allocator<CCmdBase> allocator_type;
    static CCmdBase * GetObject(){return 0;}
    static void PutObject(CCmdBase * p){}
    std::string ToString() const{return "";}
};

struct CCmdSock
{
    typedef std::allocator<CCmdSock> allocator_type;
    typedef std::vector<char> buffer;
    //constants
    static const int RET_IO_ERROR = -2;     //io����
    static const int RET_IO_RETRY = -1;     //io��Ҫ����
    static const int RET_COMPLETE = 0;      //������ɻ����������cmd
public:
    //functions
    static CCmdSock * GetObject(){return 0;}
    static void PutObject(CCmdSock * p){}
    CCmdSock()
        : sock_(0)
        , eventFlags_(0)
    {}
    int Fd() const{return 0;}
    bool Acceptable(){return false;}
    CCmdSock * Accept(){return 0;}
    void Close(){}
    CSocket & Socket(){return *sock_;}
    //�������ݣ�decode cmd
    //return: RET_xxx
    //���������ͨ��cmd����
    int RecvCmd(CCmdBase *& cmd){return RET_IO_RETRY;}
    //���ͻ��������
    //return: RET_xxx
    int SendBuffer(){return RET_COMPLETE;}
    //��buf���뷢�ͻ���
    void AddSendBuf(buffer & buf){}
    //��ȡ/���ö�д�¼���־
    bool WriteEvent() const{return (eventFlags_ & CFdEvent::EVENT_WRITE);}
    bool ReadEvent() const{return (eventFlags_ & CFdEvent::EVENT_READ);}
    void EventFlags(U32 ev){eventFlags_ = ev;}
    std::string ToString() const{return "";}
private:
    //members
    CSocket * sock_;
    U32 eventFlags_;
};

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef CCmdSock __CmdSock;
typedef CSharedPtr<__CmdSock> __SockPtr;
typedef CFdSockMap<__CmdSock, __SockPtr> __FdSockMap;
typedef __FdEventQue::container_type __FdEventList;
typedef std::vector<int> __FdList;
typedef std::vector<__SockPtr> __SockPtrList;
typedef Tools::CTriple<CCmdBase *, int, __SockPtr> __CmdTriple;
typedef CLockQueue<__CmdTriple> __QueryCmdQue;

NS_SERVER_END

#endif


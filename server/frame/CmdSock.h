#ifndef DOZERG_COMMAND_SOCK_H_20071220
#define DOZERG_COMMAND_SOCK_H_20071220

/*
    �����������ؽṹ
        CFdEvent
        CCmdBuf
        CCmdSock
        CUdpCmdData
//*/

#include <common/impl/Config.h>
#include <vector>
#include <list>
#include <algorithm>
#include <common/Tools.h>
#include <common/Sockets.h>
#include <common/Mutex.h>
#include <server/Command.h>

NS_SERVER_BEGIN

struct CFdEvent
{
    static const int EVENT_READ = 1;
    static const int EVENT_WRITE = 2;
    static const int & ExtractFd(const CFdEvent & fdEvent){
        return fdEvent.fd_;
    }
    CFdEvent(int f,int e)
        : fd_(f)
        , event_(e)
    {}
    bool Readable() const{return (event_ & EVENT_READ);}
    bool Writable() const{return (event_ & EVENT_WRITE);}
    //members:
    int fd_,event_;
};

struct CCmdBuf
{
	typedef std::vector<char> buffer_type;
    buffer_type Buf;
    bool        UseHttp;
};

class CCmdSock : public CTcpConnSocket
{
	typedef CCmdSock                __Myt;
public:
    typedef CCmdBuf::buffer_type    buffer_type;
    typedef std::list<buffer_type>  buffer_list;
	typedef std::allocator<__Myt>       allocator_type;
    typedef CGuard<CMutex>          guard_type;
    //Retrun Values
    static const int RET_CONTINUE = 0;
    static const int RET_COMPLETE = 1;
    static const int RET_INCOMPLETE = 2;
    static const int RET_CLOSED = 3;
    static const int RET_ERROR = 4;
    static const int RET_CMD_ERROR = 5;
    //Recv Status
    static const int STATUS_START = 0;
    static const int STATUS_NO_HTTP_BODY = 1;
    static const int STATUS_HTTP = 2;
    static const int STATUS_HTTP_HEAD = 3;
    static const int STATUS_HTTP_BODY = 4;
    static __Myt * GetObject();
    static void PutObject(__Myt *& p);
    CCmdSock();
    //(cmdtype = -1) for all commands.
    int RecvCommand(bool block = false,int cmdtype = -1);
    int SendCommand(const buffer_type & data,U32 timeoutMs = U32(-1));
    //��data����send_data_
    //����setSendFlag_Ϊtrue������������ǰ��ֵ
    //end��ʾ���뵽����β��������ͷ
    bool PutSendData(buffer_type & data,bool end = true);
    //ȡ��һ��Ҫ���͵����ݣ�����data��
    //�����Ƿ�ȡ��(�Ƿ��Ѿ�û����Ҫ���͵�����)
    //���û�������ˣ���setSendFlag_���ó�sendflag
    bool GetSendData(buffer_type & data,bool sendflag);
    //����recv_data_��������ResetRecv()
    void ExportCmdData(CCmdBuf & cmdbuf);
    //����recv_left_,recv_data_��recvStatus_
    void ResetRecv();
private:
    bool findHttpHead() const;
    //δ�ҵ�����0
    //�ҵ�����(��ʵ�������ʼλ��)
    size_t findHttpEnd(size_t from) const;
    void checkHttpEnd(size_t oldsz);
    bool validateCmdHead(int cmdtype);
    //members:
    U32         recv_left_;
    buffer_type recv_data_;
    int         recvStatus_ ;
    CMutex      send_lock_;
    bool        setSendFlag_;   //�Ƿ�������poll��epoll�Ŀ�д��־
    buffer_list send_data_;
};

class CUdpCmdData
{
    typedef CCmdSock::buffer_type           __Buf;
    typedef std::pair<__Buf *,CSockAddr>    __BufInfo;
	typedef std::vector<__BufInfo>          __BufVec;
	typedef std::allocator<__Buf>               __BufAlloc;
    static __Buf * getBuf(){
        __Buf * ret = __BufAlloc().allocate(1);
        return new (ret) __Buf;
    }
    static void putBuf(__BufInfo & p){
        Tools::Destroy(p.first,__BufAlloc());
    }
public:
    //��������������Clear()
    void AddCmdBuffer(__Buf & buf,const CSockAddr & peer){
        __Buf * b = getBuf();
        b->swap(buf);
        bufs_.push_back(__BufInfo(b,peer));
    }
    void Clear(){
        std::for_each(bufs_.begin(),bufs_.end(),putBuf);
        bufs_.clear();
    }
    size_t Size() const{return bufs_.size();}
    const __Buf & Buffer(size_t index) const{
        return *bufs_[index].first;
    }
    const CSockAddr & Peer(size_t index) const{
        return bufs_[index].second;
    }
private:
    __BufVec    bufs_;
};

NS_SERVER_END

#endif

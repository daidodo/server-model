#ifndef DOZERG_RECV_HELPER_H_20120201
#define DOZERG_RECV_HELPER_H_20120201

#include <SharedPtr.h>      //CAnyPtr
#include <Sockets.h>        //CSockAddr
#include "Events.h"

NS_SERVER_BEGIN

class CSockSession;

enum ECheckDataRet
{
    RR_COMPLETE,
    RR_NEED_MORE,
    RR_ERROR
};

class CSockHanle
{
    CSockSession & sock_;
    const CSockAddr & udpClientAddr_;
public:
    CSockHanle(CSockSession & sock, const CSockAddr & udpClientAddr)
        : sock_(sock)
        , udpClientAddr_(udpClientAddr)
    {}
    bool AddOutBuf(__Buffer & buf);
    std::string ToString() const;
};

struct CRecvHelper
{
    typedef std::pair<ECheckDataRet, size_t> __OnDataArriveRet;
    typedef std::pair<bool, CAnyPtr> __HandleDataRet;
    //functions
    virtual ~CRecvHelper(){}
    //获取初始接收字节数(>0)
    virtual size_t InitRecvSize() const = 0;
    //检查input的数据是否正确和完整, 不做数据处理
    //return:   first                       second
    //          RR_COMPLETE-数据接收完整    多余数据长度
    //          RR_NEED_MORE-需要更多数据   需要数据长度
    //          RR_ERROR-出错               无用
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const = 0;
    //处理input的数据: consume, decode, show, etc.
    //cmd: 返回解码成功的cmd，如果没有可以不用
    //return: true-正常; false-出错，关闭连接
    virtual bool HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const = 0;
    //释放命令函数
    virtual void ReleaseCmd(const CAnyPtr & cmd) const;
    //处理命令
    //cmd: 待处理的命令
    //handle: 与连接和对方地址有关的信息
    //return: 后续事件，EVENT_CLOSE-关闭连接，EVENT_IN-input事件，EVENT_OUT-output事件
    virtual __Events ProcessCmd(const CAnyPtr & cmd, CSockHanle & handle) const;
    //其他
    virtual std::string ToString() const;
};

NS_SERVER_END

#endif


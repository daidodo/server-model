#ifndef DOZERG_RECV_HELPER_H_20120201
#define DOZERG_RECV_HELPER_H_20120201

#include <sstream>
#include <SharedPtr.h>
#include "Events.h"

NS_SERVER_BEGIN

class CCmdBase;

enum ECheckDataRet
{
    RR_COMPLETE,
    RR_NEED_MORE,
    RR_ERROR
};

struct CRecvHelper
{
    typedef std::pair<ECheckDataRet, size_t> __OnDataArriveRet;
    typedef std::pair<bool, CAnyPtr> __HandleDataRet;
    //functions
    virtual ~CRecvHelper(){}
    //获取初始接收字节数(>0)
    virtual size_t InitRecvSize() const = 0;
    //检查input的数据是否正确和完整
    //不做数据处理
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
    virtual void ReleaseCmd(const CAnyPtr & cmd) const{};
    //处理命令
    virtual __Events ProcessCmd(const CAnyPtr & cmd) const{return 0;}
    //其他
    virtual std::string ToString() const{
        std::ostringstream oss;
        oss<<"{@"<<this<<"}";
        return oss.str();
    }
};

NS_SERVER_END

#endif


#ifndef DOZERG_RECV_HELPER_H_20120201
#define DOZERG_RECV_HELPER_H_20120201

#include <impl/Config.h>

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
    //functions
    virtual ~CRecvHelper(){}
    //获取初始接收字节数(>0)
    virtual size_t InitRecvSize() const = 0;
    //检查input的数据是否正确和完整
    //不做数据处理
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const = 0;
    //处理input的数据
    //return: true-正常; false-出错，关闭连接
    virtual bool HandleData(const char * buf, size_t sz) const = 0;
    //解析命令处理函数
    virtual CCmdBase * DecodeCmd(const char * buf, size_t sz) const = 0;
    //释放命令处理函数
    virtual void ReleaseCmd(CCmdBase * cmd) const = 0;
};

NS_SERVER_END

#endif


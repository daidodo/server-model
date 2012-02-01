#ifndef DOZERG_RECV_HELPER_H_20120201
#define DOZERG_RECV_HELPER_H_20120201

#include <impl/Config.h>

NS_SERVER_BEGIN

//user-defined call-backs
enum ECheckDataRet
{
    RR_COMPLETE,
    RR_NEED_MORE,
    RR_ERROR
};

typedef std::pair<ECheckDataRet, size_t> __OnDataArriveRet;
typedef __OnDataArriveRet (*__OnDataArrive)(const char *, size_t);
typedef void * (*__DecodeCmd)(const char *, size_t);
typedef void (*__ReleaseCmd)(void * cmd);

struct CRecvHelper
{
    //functions
    CRecvHelper(size_t initRecvSz)
        : initSz_(initRecvSz)
    {}
    virtual ~CRecvHelper(){}
    bool IsTcpValid() const{return initSz_;}
    std::string ToString() const;
    //获取初始接收字节数
    size_t InitRecvSize() const{return initSz_;}
    //接收数据处理函数
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const{
        return __OnDataArriveRet(RR_ERROR, 0);
    }



    //设置/获取解析命令处理函数
    void DecodeCmd(__DecodeCmd decodeCmd){decodeCmd_ = decodeCmd;}
    __DecodeCmd DecodeCmd() const{return decodeCmd_;}
    //设置/获取释放命令处理函数
    void ReleaseCmd(__ReleaseCmd releaseCmd){releaseCmd_ = releaseCmd;}
    __ReleaseCmd ReleaseCmd() const{return releaseCmd_;}
private:
    //members
    const size_t initSz_;
    __DecodeCmd decodeCmd_;
    __ReleaseCmd releaseCmd_;
};

NS_SERVER_END

#endif


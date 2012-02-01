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
    //��ȡ��ʼ�����ֽ���
    virtual size_t InitRecvSize() const = 0;
    //�������ݴ�����
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const = 0;
    //�����������
    virtual CCmdBase * DecodeCmd(const char * buf, size_t sz) const = 0;
    //�ͷ��������
    virtual void ReleaseCmd(CCmdBase * cmd) const = 0;
};

NS_SERVER_END

#endif


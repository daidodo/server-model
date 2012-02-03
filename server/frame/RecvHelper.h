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
    //��ȡ��ʼ�����ֽ���(>0)
    virtual size_t InitRecvSize() const = 0;
    //���input�������Ƿ���ȷ������, �������ݴ���
    //return:   first                       second
    //          RR_COMPLETE-���ݽ�������    �������ݳ���
    //          RR_NEED_MORE-��Ҫ��������   ��Ҫ���ݳ���
    //          RR_ERROR-����               ����
    virtual __OnDataArriveRet OnDataArrive(const char * buf, size_t sz) const = 0;
    //����input������: consume, decode, show, etc.
    //cmd: ���ؽ���ɹ���cmd�����û�п��Բ���
    //return: true-����; false-�����ر�����
    virtual bool HandleData(const char * buf, size_t sz, CAnyPtr & cmd) const = 0;
    //�ͷ������
    virtual void ReleaseCmd(const CAnyPtr & cmd) const;
    //��������
    //cmd: �����������
    //handle: �����ӺͶԷ���ַ�йص���Ϣ
    //return: �����¼���EVENT_CLOSE-�ر����ӣ�EVENT_IN-input�¼���EVENT_OUT-output�¼�
    virtual __Events ProcessCmd(const CAnyPtr & cmd, CSockHanle & handle) const;
    //����
    virtual std::string ToString() const;
};

NS_SERVER_END

#endif


#ifndef DZ_LOGSYS_LOACAL_LOGGER_20071023
#define DZ_LOGSYS_LOACAL_LOGGER_20071023

/*
    �û���ֱ��ʹ�õ���־����
        CGlobalLogger   ���̰߳�ȫ��logger,�����κλ�����ʹ��
        CLocalLogger    ���̵߳�logger,ֻ���ڵ����������ڻ��̻߳�����ʹ��
    History:
        20081014    ����
//*/

#include "logsys_logstream.h"
#include "logsys_logformat.h"

DZLOG_BEGIN

class CGlobalLogger : public DZLOG_IMPL::CLogFormat
{
    NS_SERVER::CMutex mutex_;
public:
    explicit CGlobalLogger(const char * head = "")
        : CLogFormat(head)
    {}
    void Lock(){mutex_.Lock();}
    void Unlock(){mutex_.Unlock();}
};

struct CLocalLogger : public DZLOG_IMPL::CLogFormat
{
    explicit CLocalLogger(const char * head = "")
        : CLogFormat(head)
    {}
    void Lock(){}   //Ϊ��ʹ����һ��
    void Unlock(){}
};

struct CScopeLogger : public DZLOG_IMPL::CLogFormat
{
    explicit CScopeLogger(const char * head = "")
        : CLogFormat(head)
    {}
    void Lock(){}   //Ϊ��ʹ����һ��
    void Unlock(){}
    void FlushStream(){}    //��������ʱ��ˢд������־
};

DZLOG_END

#endif

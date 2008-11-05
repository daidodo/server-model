#ifndef DZ_LOGSYS_LOACAL_LOGGER_20071023
#define DZ_LOGSYS_LOACAL_LOGGER_20071023

/*
    用户可直接使用的日志对象
        CGlobalLogger   多线程安全的logger,能在任何环境下使用
        CLocalLogger    单线程的logger,只能在单个函数以内或单线程环境下使用
    History:
        20081014    增加
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
    void Lock(){}   //为了使用上一致
    void Unlock(){}
};

struct CScopeLogger : public DZLOG_IMPL::CLogFormat
{
    explicit CScopeLogger(const char * head = "")
        : CLogFormat(head)
    {}
    void Lock(){}   //为了使用上一致
    void Unlock(){}
    void FlushStream(){}    //在析构的时候刷写所有日志
};

DZLOG_END

#endif

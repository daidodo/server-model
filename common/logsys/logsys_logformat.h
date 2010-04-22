#ifndef DZ_LOGSYS_LOG_FORMAT_20071023
#define DZ_LOGSYS_LOG_FORMAT_20071023

#include <vector>
#include <string>
#include <sstream>
#include <time.h>
//#include <pthread.h>
#include <common/Tools.h>
#include "logsys_logstream.h"

IMPL_BEGIN

//������־�ĸ�����Ϣ,������ʽ
//����ʱ��,�߳�ID,����,��Ϣͷ(������������)
//������ʱ��:TRACE,DEBUG,INFO,WARN,ERROR,FATAL,OFF
class CLogFormat : public CLogStream
{
    const char * head_;
public:
    explicit CLogFormat(const char * h,bool focus)
        : CLogStream(focus)
        , head_(h)
    {}
    CLogFormat & HeadInfo(int level){
        *this<<NS_SERVER::Tools::TimeString(time(0),CLogStream::timeFormat())
            //<<"["<<pthread_self()<<"]"
            <<"["<<levelName(level)<<"] ";
        if(head_)
            *this<<head_;
        *this<<" - ";
        return *this;
    }
private:
    const char * levelName(int lv) const{
        const int LEVEL_COUNT = 6;
        const char * LEVEL[LEVEL_COUNT] = {
            "TRACE","DEBUG","INFO ","WARN ","ERROR","FATAL"
        };
        return (lv >= 0 && lv < LEVEL_COUNT ? LEVEL[lv] : "UNDEF");
    }
};

IMPL_END

#endif

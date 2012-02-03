#ifndef DZ_LOGSYS_LOG_STREAM_20071023
#define DZ_LOGSYS_LOG_STREAM_20071023

#include <string>
#include <sstream>
#include "logsys_singlewriter.h"

IMPL_BEGIN

//负责对<<进行重载
//CSingleWriter的外壳
class CLogStream
{
    CSingleWriter & writer_;
    std::string     msg_;
public:
    CLogStream()
        : writer_(CSingleWriter::Instance())
    {}
    ~CLogStream(){FlushStream();}
    template<class T>
    CLogStream & operator <<(const T & s){
        std::ostringstream oss;
        oss<<s;
        msg_ += oss.str();
        return *this;
    }
    void FlushStream(){
        if(!msg_.empty()){
            writer_.LockWrite(msg_.c_str(),msg_.length());
            msg_.clear();
        }
    }
    int Level() const{return writer_.Level();}
protected:
    const char * timeFormat() const{return writer_.TimeFormat();}
};

IMPL_END

#endif

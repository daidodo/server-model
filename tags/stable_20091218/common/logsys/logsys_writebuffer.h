#ifndef DZ_LOGSYS_WRITE_BUFFER_20071019
#define DZ_LOGSYS_WRITE_BUFFER_20071019

#include <algorithm>
#include <new>
#include <cstring>          //memcpy
#include "logsys_writer.h"

IMPL_BEGIN

//写缓冲,设置最佳的缓冲大小
//选择合适的时机进行文件写入
class CWriteBuffer : public CWriter
{
    char *      buffer_;
    size_t      buf_sz_,cur_;
public:
    CWriteBuffer()
        :buffer_(0),buf_sz_(0),cur_(0)
    {}
    ~CWriteBuffer(){
        if(buffer_){
            delete [] buffer_;
            buffer_ = 0;
        }
    }
protected:
    bool setBuffer(size_t sz){
        if(sz){
            buf_sz_ = sz;
            buffer_ = new (std::nothrow) char[buf_sz_];
        }
        return buffer_ != 0;
    }
    bool flushBuffer(){
        if(!cur_ || !buffer_)
            return true;
        bool ret = CWriter::writeFile(buffer_,cur_);
        cur_ = 0;
        return ret;
    }
    bool writeBuffer(const char * buf,size_t sz){
        assert(buf && sz);
        assert(cur_ <= buf_sz_);
        assert(buffer_ && "logsys not initialized");
        if(cur_ + sz < buf_sz_){
            getData(buf,sz);
        }else{
            while(sz > 0){
                size_t cp_sz = std::min(buf_sz_ - cur_,sz);
                getData(buf,cp_sz);
                buf += cp_sz;
                sz -= cp_sz;
                if(cur_ == buf_sz_ && !flushBuffer())
                    return false;
            }
        }
        return true;
    }
    void closeFile(){
        flushBuffer();
        close(CWriter::Fd());
        CWriter::setFd(-1);
    }
private:
    CWriteBuffer(const CWriteBuffer &);
    CWriteBuffer & operator =(const CWriteBuffer &);
    void getData(const char * buf,size_t sz){
        assert(buf && sz);
        assert(cur_ + sz <= buf_sz_);
        memcpy(buffer_ + cur_,buf,sz);
        cur_ += sz;
    }
};

IMPL_END

#endif

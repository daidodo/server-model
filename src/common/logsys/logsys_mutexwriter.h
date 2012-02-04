#ifndef DZ_LOGSYS_MUTEX_WRITER_20071020
#define DZ_LOGSYS_MUTEX_WRITER_20071020

#include <Mutex.h>
#include "logsys_writebuffer.h"
#include "logsys_filemanager.h"

IMPL_BEGIN

//对写操作进行互斥管理
//尽量选择高效的互斥方式
class CMutexWriter : public CWriteBuffer
{
    typedef NS_SERVER::CMutex               lock_type;
    typedef NS_SERVER::CGuard<lock_type>    guard_type;
    lock_type       mutex_;
    size_t          total_; //累计已写入的字节数
    CFileManager    fm_;
    bool            quickFlush_;
public:
    CMutexWriter():total_(0){}
    ~CMutexWriter(){
        guard_type guard(mutex_);
        CWriteBuffer::flushBuffer();
    }
    void LockWrite(const char * buf,size_t sz){
        guard_type guard(mutex_);
        if(CWriteBuffer::writeBuffer(buf,sz)){
            if((total_ += sz) >= fm_.FileSize()){
                CWriteBuffer::closeFile();
                fm_.RollFile();
                int fd = fm_.OpenLogFile();
                assert(fd > 0 && "CMutexWriter::LockWrite");
                CWriter::setFd(fd);
                total_ = 0;
            }
            if(quickFlush_)
                CWriteBuffer::flushBuffer();
        }
    }
protected:
    void configMWriter(const CConfigItems & item){
        fm_.Config(item);
        int fd = -1;
        fd = fm_.OpenLogFile();
        assert(fd > 0 && "cannot open log file");
        CWriter::setFd(fd);
        quickFlush_ = item.quickFlush_;
        CWriteBuffer::setBuffer(item.buffersz_);
        struct stat fs;
        if(fstat(fd,&fs) == 0)
            total_ = fs.st_size;
    }
};

IMPL_END

#endif

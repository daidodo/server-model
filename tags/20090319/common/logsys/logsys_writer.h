#ifndef DZ_LOGSYS_WRITER_20071019
#define DZ_LOGSYS_WRITER_20071019

#include <cassert>
#include <stdio.h>
#include <unistd.h>
#include "logsys_namespace.h"

IMPL_BEGIN

//���ļ�д����з�װ,��ʱд��
//������������д�ļ���ʽʵ��
//���ÿ��ǻ���,����һ������
class CWriter
{
    int fd_;
public:
    CWriter():fd_(-1){}
    ~CWriter(){
        if(fd_ > 0)
            close(fd_);
    }
protected:
    void setFd(int fd){fd_ = fd;}
    int Fd() const{return fd_;}
    bool writeFile(const char * buf,size_t sz){
        assert(fd_ > 0 && buf && "CWriter::Write");
        return size_t(write(fd_,buf,sz)) == sz;
    }
};

IMPL_END

#endif

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>        //std::runtime_error

#include <Sockets.h>
#include <Tools.h>
#include "FileDesc.h"

NS_SERVER_BEGIN

//struct IFileDesc

#define __CASE_GET_OBJECT(id, type) \
        case id:    \
            ret = type::allocator_type().allocate(1);   \
            return new (ret) type

#define __CASE_PUT_OBJECT(id, type) \
        case id:{   \
            type * t = dynamic_cast<type *>(p); \
            Tools::Destroy(t, type::allocator_type());  \
            break;}

IFileDesc * IFileDesc::GetObject(EFileDescType type)
{
    IFileDesc * ret = 0;
    switch(type){
        __CASE_GET_OBJECT(FD_FILE, CFile);
        __CASE_GET_OBJECT(FD_TCP_LISTEN, CListenSocket);
        __CASE_GET_OBJECT(FD_TCP_CONN, CTcpConnSocket);
        __CASE_GET_OBJECT(FD_UDP, CUdpSocket);
    }
    throw std::runtime_error("file desc type invalid");
}

void IFileDesc::PutObject(IFileDesc * p)
{
    if(!p)
        return;
    switch(p->type_){
        __CASE_PUT_OBJECT(FD_FILE, CFile);
        __CASE_PUT_OBJECT(FD_TCP_LISTEN, CListenSocket);
        __CASE_PUT_OBJECT(FD_TCP_CONN, CTcpConnSocket);
        __CASE_PUT_OBJECT(FD_UDP, CUdpSocket);
    }
}

#undef __CASE_GET_OBJECT
#undef __CASE_PUT_OBJECT

IFileDesc::~IFileDesc()
{
    Close();
}

bool IFileDesc::SetBlock(bool on)
{
    if(!IsValid())
        return false;
    int oldflag = fcntl(fd_, F_GETFL);
    if(oldflag == -1)
        return false;
    int newflag = (on ? oldflag & ~O_NONBLOCK : oldflag | O_NONBLOCK);
    if(oldflag == newflag)
        return true;
    if(fcntl(fd_, F_SETFL, newflag) < 0)
        return false;
    return true;
}

void IFileDesc::Close()
{
    if(IsValid()){
        close(fd_);
        fd_ = INVALID_FD;
    }
}

std::string IFileDesc::ToString() const
{
    std::ostringstream oss;
    oss<<"{fd_="<<fd_
        <<"}";
    return oss.str();
}

//struct CFile

bool CFile::Open(const std::string & pathname, int flags, mode_t mode)
{
    if(IsValid())
        Close();
    if(pathname.empty())
        return false;
    fd_ = open(pathname.c_str(), flags, mode);
    pathname_ = pathname;
    return IsValid();
}

ssize_t CFile::Read(char * buf, size_t size)
{
    if(!IsValid())
        return false;
    return read(fd_, buf, size);
}

ssize_t CFile::Write(const char * buf, size_t size)
{
    if(!IsValid())
        return false;
    return write(fd_, buf, size);
}

std::string CFile::ToString() const
{
    std::ostringstream oss;
    oss<<"{(CFile)base="<<IFileDesc::ToString()
        <<", pathname_="<<Tools::Dump(pathname_)
        <<"}";
    return oss.str();
}

NS_SERVER_END

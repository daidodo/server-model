#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "FileDesc.h"

NS_SERVER_BEGIN

//struct IFileDesc

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

NS_SERVER_END

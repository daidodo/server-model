#ifndef DOZERG_FILE_DESC_H_20120119
#define DOZERG_FILE_DESC_H_20120119

#include <sys/types.h>  //mode_t
#include <errno.h>
#include <string>
#include <vector>

#include <Tools.h>

NS_SERVER_BEGIN

struct IFileDesc
{
    static const int INVALID_FD = -1;
    static std::string ErrMsg(){return Tools::ErrorMsg(errno);}
    IFileDesc():fd_(INVALID_FD){}
    virtual ~IFileDesc() = 0;
    int Fd() const{return fd_;}
    bool IsValid() const{return fd_ != INVALID_FD;}
    bool SetBlock(bool on = true);
    void Close();
private:
    IFileDesc(const IFileDesc &);
    IFileDesc & operator =(const IFileDesc &);
protected:
    int fd_;
};

struct CFile : public IFileDesc
{
    //functions
    CFile(){}
    CFile(const std::string & pathname, int flags, mode_t mode){
        Open(pathname, flags, mode);
    }
    const std::string & Pathname() const{return pathname_;}
    bool Open(const std::string & pathname, int flags, mode_t mode);
    //从文件最多读取size字节数据放到buf里
    //return: -1:出错; +n:实际读出的字节数
    ssize_t Read(char * buf, size_t size);
    //append: true-追加到已有数据末尾; false-覆盖已有数据
    //return: 是否出错
    bool Read(std::vector<char> & buf, size_t size, bool append){
        return readData(buf, size, append);
    }
    bool Read(std::string & buf, size_t size, bool append){
        return readData(buf, size, append);
    }
    //将buf数据写入文件
    //size: buf数据长度
    //return: -1-出错; +n-实际写入的字节数
    ssize_t Write(const char * buf, size_t size);
    ssize_t Write(const std::vector<char> & buf){
        return Write(&buf[0], buf.size());
    }
    ssize_t Write(const std::string & buf){
        return Write(&buf[0], buf.size());
    }
private:
    template<class Buf>
    bool readData(Buf & buf, size_t size, bool append){
        if(!IsValid())
            return false;
        size_t from = (append ? buf.size() : 0);
        buf.resize(from + size);
        ssize_t ret = Read(&buf[from], size);
        buf.resize(from + (ret > 0 ? ret : 0));
        return (ret >= 0);
    }
    //members
    std::string pathname_;
};

NS_SERVER_END

#endif


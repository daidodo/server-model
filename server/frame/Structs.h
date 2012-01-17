#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

NS_SERVER_BEGIN

struct CFdEvent
{
    static const int EVENT_READ = 1;
    static const int EVENT_WRITE = 2;
    static const int EVENT_CLOSE = 4;
    static const int EVENT_EXCL = 8;
    static const int & ExtractFd(const CFdEvent & fe){return fe.fd_;}
    CFdEvent(int f, int e)
        : fd_(f)
        , event_(e)
    {}
    int Fd() const{return fd_;}
    bool Readable() const{return (event_ & EVENT_READ);}
    bool Writable() const{return (event_ & EVENT_WRITE);}
    bool Closable() const{return (event_ & EVENT_CLOSE);}
    bool Exclusive() const{return (event_ & EVENT_EXCL);}
private:
    //members:
    int fd_;
    int event_;
};

struct CCmdSock
{
    typedef std::allocator<CCmdSock> allocator_type;
    //functions
    bool Acceptable(){return false;}
    int Fd() const{return 0;}
    std::string ToString() const{return "";}
    void Close(){}
};

NS_SERVER_END

#endif


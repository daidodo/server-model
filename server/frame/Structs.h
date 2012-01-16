#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

NS_SERVER_BEGIN

class CFdEvent
{
    static const int EVENT_READ = 1;
    static const int EVENT_WRITE = 2;
    static const int EVENT_CLOSE = 4;
public:
    static const int & ExtractFd(const CFdEvent & fe){return fe.Fd();}
    CFdEvent(int f, int e)
        : fd_(f)
        , event_(e)
    {}
    int Fd() const{return fd_;}
    bool Readable() const{return (event_ & EVENT_READ);}
    bool Writable() const{return (event_ & EVENT_WRITE);}
    bool Closable() const{return (event_ & EVENT_CLOSE);}
    //members:
private:
    int fd_;
    int event_;
};

class CCmdSock
{
    //functions
public:
    bool Acceptable(){return false;}
};

NS_SERVER_END

#endif


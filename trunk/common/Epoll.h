#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <sys/epoll.h>
#include <cstring>          //memset
#include <vector>
#include <set>
#include <common/FdMap.h>   //CFdMap

/*
    epoll操作封装,采用ET模式
        CEpoll
//*/

NS_SERVER_BEGIN

class CEpoll
{
    typedef struct epoll_event  __Event;
    typedef __DZ_SET(int)       __FdSet;
public:
    typedef __FdSet::iterator   iterator;
    typedef iterator            const_iterator;
private:
    class __EventHelper{
        const __Event & ev_;
        U32 &           fdtime_;    //s
        U32             timeout_;   //s
    public:
        __EventHelper(const __Event & ev,U32 & ft,U32 tt)
            : ev_(ev)
            , fdtime_(ft)
            , timeout_(tt)
        {}
        bool Invalid() const{return ev_.data.fd < 0;}
        bool Readable() const{return ev_.events & EPOLLIN;}
        bool Writable() const{return ev_.events & EPOLLOUT;}
        bool IsExpired(U32 curtime) const{
            return isTimeout(fdtime_,timeout_,curtime);
        }
        bool CanExpire() const{return canExpire(fdtime_,timeout_);}
        void SetTime(U32 curtime){fdtime_ = curtime;}
        bool Error() const{
            return ev_.events & EPOLLERR || ev_.events & EPOLLHUP;
        }
        int FD() const{return ev_.data.fd;}
        U32 ExpireTime() const{
            return expireTime(fdtime_,timeout_);
        }
    };
    static bool canExpire(U32 fdtime,U32 timeout){
        return fdtime && timeout;
    }
    static U32 expireTime(U32 fdtime,U32 timeout){
        return (canExpire(fdtime,timeout) ? fdtime + timeout : U32(-1));
    }
    static bool isTimeout(U32 fdtime,U32 timeout,U32 curtime){
        return expireTime(fdtime,timeout) < curtime;
    }
public:
    CEpoll()
        : epollfd_(-1)
        , maxsz_(0)
        , fdtimeout_(0)
        , epollTimeout_(3000)
    {}
    ~CEpoll(){Destroy();}
    bool IsValid() const{return epollfd_ > 0;}
    //如果fdtimeout_设置为0，表示所有fd永不超时
    void SetFdTimeout(U32 s){fdtimeout_ = s;}
    U32 GetFdTimeout() const{return fdtimeout_;}
    void SetEpollTimeout(int ms){epollTimeout_ = ms;}
    int GetEpollTimeout() const{return epollTimeout_;}
    size_t MaxFdSize() const{return maxsz_;}
    size_t FdSize() const{return fdSet_.size();}
    size_t Size() const{return revents_.size();}
    bool CanTimeout() const{return fdtimeout_;}
    bool Create(int max_size){
        if(!IsValid())
            epollfd_ = epoll_create(max_size);
        return IsValid();
    }
    void Destroy(){
        if(IsValid()){
            close(epollfd_);
            epollfd_ = -1;
        }
    }
    //flags将被加上EPOLLET
    bool AddFd(int fd,int flags,U32 curtime){
        __Event ev;
        memset(&ev,0,sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd =fd;
        if(epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev) < 0)
            return false;
        fdTime_[fd] = curtime;
        fdSet_.insert(fd);
        return true;
    }
    //del表示是否需要从epoll里删除fd
    bool RemoveFd(int fd,bool del = false){
        if(del && epoll_ctl(epollfd_,EPOLL_CTL_DEL,fd,0) < 0)
            return false;
        fdSet_.erase(fd);
        return true;
    }
    template<class ForwardIter>
    void RemoveFd(ForwardIter first,ForwardIter last,bool del = false){
        for(;first != last;++first)
            RemoveFd(*first,del);
    }
    //flags将被加上EPOLLET
    bool ModifyFd(int fd,int flags){
        __Event ev;
        memset(&ev,0,sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd =fd;
        return (epoll_ctl(epollfd_,EPOLL_CTL_MOD,fd,&ev) == 0);
    }
    bool Wait(){
        size_t sz = FdSize();
        revents_.resize(sz);
        if(sz){
            int n = epoll_wait(epollfd_,&revents_[0],sz,epollTimeout_);
            if(n < 0)
                return false;
            revents_.resize(n);
        }else
            usleep(std::min(epollTimeout_ * 1000,5000000));
        return true;
    }
    __EventHelper operator [](size_t index){
        return __EventHelper(revents_[index],fdTime_[revents_[index].data.fd],fdtimeout_);
    }
    bool IsFdTimeout(int fd,U32 curtime) const{
        return isTimeout(fdTime_[fd],fdtimeout_,curtime);
    }
    U32 ExpireTime(int fd) const{
        return expireTime(fdTime_[fd],fdtimeout_);
    }
    const_iterator begin() const{return fdSet_.begin();}
    const_iterator end() const{return fdSet_.end();}
private:
    int                     epollfd_;
    int                     maxsz_;
    __DZ_VECTOR(__Event)    revents_;
    CFdMap<U32>             fdTime_;        //s,每个fd的上次活跃时间
    __FdSet                 fdSet_;         //当前处理的fd集合
    U32                     fdtimeout_;     //s,fd的超时时间
    int                     epollTimeout_;  //ms,epoll的阻塞时间
};

NS_SERVER_END

#endif

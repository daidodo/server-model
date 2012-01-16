#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <sys/epoll.h>
#include <vector>
#include <set>
#include <cstring>   //memset
#include <FdMap.h>   //CFdMap

/*
    epoll操作封装,采用ET模式
        CEpollEvent
        CEpoll
//*/

NS_SERVER_BEGIN

struct CEpollEvent{
    typedef struct epoll_event __Event;
    //functions
    CEpollEvent(const __Event & ev, U32 & ft, U32 tt)
        : ev_(ev)
        , fdtime_(ft)
        , timeout_(tt)
    {}
    int Fd() const{return ev_.data.fd;}
    void SetTime(U32 curtime){fdtime_ = curtime;}
    bool Invalid() const{return ev_.data.fd < 0;}
    bool Readable() const{return ev_.events & EPOLLIN;}
    bool Writable() const{return ev_.events & EPOLLOUT;}
    bool Error() const{
        return ev_.events & EPOLLERR || ev_.events & EPOLLHUP;
    }
    bool CanExpire() const{return canExpire(fdtime_,timeout_);}
    bool IsExpired(U32 curtime) const{
        return isTimeout(fdtime_,timeout_,curtime);
    }
    U32 ExpireTime() const{
        return expireTime(fdtime_,timeout_);
    }
private:
    static bool canExpire(U32 fdtime,U32 timeout){
        return fdtime && timeout;
    }
    static U32 expireTime(U32 fdtime,U32 timeout){
        return (canExpire(fdtime,timeout) ? fdtime + timeout : U32(-1));
    }
    static bool isTimeout(U32 fdtime,U32 timeout,U32 curtime){
        return expireTime(fdtime,timeout) < curtime;
    }
    //members
    const __Event & ev_;
    U32 &           fdtime_;    //s
    U32             timeout_;   //s
};

class CEpoll
{
    typedef CEpollEvent::__Event __Event;
    struct __FdInfo{
        U32 flags_;
        U32 fdtime_;
    };
public:
    CEpoll()
        : epollfd_(-1)
        , maxsz_(0)
        , fdtimeout_(0)
        , epollTimeout_(400)
    {}
    ~CEpoll(){Destroy();}
    bool IsValid() const{return epollfd_ >= 0;}
    //设置/获取fd的超时时间(秒)
    //如果fdtimeout_设置为0，表示所有fd永不超时
    void FdTimeout(U32 s){fdtimeout_ = s;}
    U32 FdTimeout() const{return fdtimeout_;}
    bool CanTimeout() const{return fdtimeout_ != 0;}
    //设置/获取epoll wait的时间(毫妙)
    void EpollTimeout(int ms){epollTimeout_ = ms;}
    int EpollTimeout() const{return epollTimeout_;}
    //获取fd个数的上限
    size_t MaxFdSize() const{return maxsz_;}
    //获取已加入epoll的fd的个数
    size_t FdSize() const{return fdSize_;}
    //获取当前有读写事件的fd的个数
    size_t Size() const{return revents_.size();}
    //创建epoll
    //max_size: fd个数上限
    bool Create(int max_size){
        if(!IsValid())
            epollfd_ = epoll_create(max_size);
        return IsValid();
    }
    //回收epoll
    void Destroy(){
        if(IsValid()){
            close(epollfd_);
            epollfd_ = -1;
        }
    }
    //添加或者修改fd和flags
    //如果fd没有，则添加
    //如果fd已有，则修改，flags是OR的关系
    //flags将被加上EPOLLET
    bool AddOrModifyFd(int fd, U32 flags, U32 curtime){
        U32 oldFlags = fdInfo_[fd].flags_;
        if(oldFlags == flags)
            return true;
        if(oldFlags)
            return ModifyFd(fd, oldFlags | flags);
        return AddFd(fd, flags, curtime);
    }
    //将fd从fdInfo_中移除
    //del:是否需要从epoll里删除fd
    bool RemoveFd(int fd, bool del = false){
        if(del && epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, 0) < 0)
            return false;
        U32 & flags = fdInfo_[fd].flags_;
        if(flags){
            flags = 0;
            --fdSize_;
        }
        return true;
    }
    template<class ForwardIter>
    void RemoveFd(ForwardIter first, ForwardIter last, bool del = false){
        for(;first != last;++first)
            RemoveFd(*first, del);
    }
    //等待epoll事件
    bool Wait(){
        revents_.resize(fdSize_);
        if(sz){
            int n = epoll_wait(epollfd_, &revents_[0], sz, epollTimeout_);
            if(n < 0)
                return false;
            revents_.resize(n);
        }else
            usleep(std::min(epollTimeout_ * 1000, 5000000));
        return true;
    }
    //根据下标获取读写事件
    //index: 范围在[0, CEpoll::Size())之间
    CEpollEvent operator [](size_t index) const{
        return CEpollEvent(revents_[index], fdInfo_[revents_[index].data.fd].fdtime_, fdtimeout_);
    }
    //获取fd是否已经超时
    //bool IsFdTimeout(int fd,U32 curtime) const{
    //    return isTimeout(fdInfo_[fd].fdtime_,fdtimeout_,curtime);
    //}
    //获取fd的超时时间点
    //U32 ExpireTime(int fd) const{
    //    return expireTime(fdInfo_[fd].fdtime_,fdtimeout_);
    //}
    //遍历获取所有加入epoll的fd
    //const_iterator begin() const{return fdSet_.begin();}
    //const_iterator end() const{return fdSet_.end();}
private:
    //将fd加入到epoll中
    //flags将被加上EPOLLET
    bool addFd(int fd, U32 flags, U32 curtime){
        __Event ev;
        memset(&ev, 0, sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd = fd;
        if(epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) < 0)
            return false;
        fdInfo_[fd].fdtime_ = curtime;
        fdInfo_[fd].flags_ = flags;
        ++fdSize_;
        return true;
    }
    //改变fd的flags
    //flags将被加上EPOLLET，并覆盖原flags
    bool modifyFd(int fd, U32 flags){
        __Event ev;
        memset(&ev, 0, sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd = fd;
        if(epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev) != 0)
            return false;
        fdInfo_[fd].flags_ = flags;
        return true;
    }
    size_t                  fdSize_;
    int                     epollfd_;
    int                     maxsz_;
    U32                     fdtimeout_;     //s,fd的超时时间
    int                     epollTimeout_;  //ms,epoll的阻塞时间
    CFdMap<__FdInfo>        fdInfo_;        //每个fd的辅助信息
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <sys/epoll.h>
#include <vector>
#include <set>
#include <cstring>   //memset
#include <FdMap.h>   //CFdMap
#include <Tools.h>   //CFdMap

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
        return (ev_.events & EPOLLERR) || (ev_.events & EPOLLHUP);
    }
    bool Expired(U32 curtime) const{
        return Tools::IsTimeout(fdtime_, curtime, timeout_);
    }
    U32 ExpireTime() const{fdtime_ + timeout_;}
private:
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
        U32 activeTime_;
    };
public:
    CEpoll()
        : epollFd_(-1)
        , maxSz_(0)
        , fdTimeoutS_(0)
        , epollTimeoutMs_(400)
    {}
    ~CEpoll(){Destroy();}
    bool IsValid() const{return epollFd_ >= 0;}
    //设置/获取fd的超时时间(秒)
    //如果fdTimeoutS_设置为0，表示所有fd永不超时
    void FdTimeout(U32 s){fdTimeoutS_ = s;}
    U32 FdTimeout() const{return fdTimeoutS_;}
    bool CanTimeout() const{return fdTimeoutS_ != 0;}
    //设置/获取epoll wait的时间(毫妙)
    void EpollTimeout(int ms){epollTimeoutMs_ = ms;}
    int EpollTimeout() const{return epollTimeoutMs_;}
    //获取fd个数的上限
    size_t MaxFdSize() const{return maxSz_;}
    //获取已加入epoll的fd的个数
    size_t FdSize() const{return fdSize_;}
    //获取当前有读写事件的fd的个数
    size_t Size() const{return revents_.size();}
    //创建epoll
    //max_size: fd个数上限
    bool Create(int max_size){
        if(!IsValid())
            epollFd_ = epoll_create(max_size);
        return IsValid();
    }
    //回收epoll
    void Destroy(){
        if(IsValid()){
            close(epollFd_);
            epollFd_ = -1;
        }
    }
    //增加fd对应的flags
    //如果fd没有flags，则新增
    //如果fd已有flags，则OR
    bool AddFlags(int fd, U32 flags, U32 curtime)
    {
        U32 & oldFlags = fdInfo_[fd].flags_;
        if(oldFlags)
            return modifyFdFlags(fd, flags, curtime);
        return addFdFlags(fd, flags, curtime);
    }
    //修改fd对应的flags
    //如果fd没有flags，则新增
    //如果fd已有flags，则覆盖
    bool ModFlags(int fd, U32 flags, U32 curtime)
    {
        U32 & oldFlags = fdInfo_[fd].flags_;
        if(oldFlags)
            return modifyFdFlags(fd, flags | oldFlags, curtime);
        return addFdFlags(fd, flags, curtime);
    }
    //将fd从Epoll中移除
    //del: 是否需要从epoll里删除fd
    bool RemoveFd(int fd, bool del = false){
        if(del && epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, 0) < 0)
            return false;
        U32 & flags = fdInfo_[fd].flags_;
        if(flags){
            flags = 0;
            assert(fdSize_ > 0);
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
    //return: true-成功; false-出错
    bool Wait(){
        revents_.resize(fdSize_);
        if(sz_){
            int n = epoll_wait(epollFd_, &revents_[0], sz, epollTimeoutMs_);
            if(n < 0)
                return false;
            revents_.resize(n);
        }else
            usleep(std::min(epollTimeoutMs_ * 1000, 500000));
        return true;
    }
    //根据下标获取读写事件
    //index: 范围在[0, CEpoll::Size())之间
    CEpollEvent operator [](size_t index) const{
        return CEpollEvent(revents_[index], fdInfo_[revents_[index].data.fd].fdtime_, fdTimeoutS_);
    }
    //获取fd是否已经超时
    //bool IsFdTimeout(int fd,U32 curtime) const{
    //    return isTimeout(fdInfo_[fd].fdtime_,fdTimeoutS_,curtime);
    //}
    //获取fd的超时时间点
    //U32 ExpireTime(int fd) const{
    //    return expireTime(fdInfo_[fd].fdtime_,fdTimeoutS_);
    //}
    //遍历获取所有加入epoll的fd
    //const_iterator begin() const{return fdSet_.begin();}
    //const_iterator end() const{return fdSet_.end();}
private:
    //将fd加入到epoll中
    //flags将被加上EPOLLET
    bool addFdFlags(int fd, U32 flags, U32 curtime){
        __Event ev;
        memset(&ev, 0, sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd = fd;
        if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) < 0)
            return false;
        assert(0 == fdInfo_[fd].flags_);
        fdInfo_[fd].flags_ = flags;
        fdInfo_[fd].activeTime_ = curtime;
        ++fdSize_;
        return true;
    }
    //改变fd的flags
    //flags将被加上EPOLLET，并覆盖原flags
    bool modifyFdFlags(int fd, U32 flags, U32 curtime){
        U32 & oldFlags = fdInfo_[fd].flags_;
        assert(0 != oldFlags);
        if(flags != oldFlags){
            __Event ev;
            memset(&ev, 0, sizeof ev);
            ev.events = flags | EPOLLET;
            ev.data.fd = fd;
            if(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev) != 0)
                return false;
            oldFlags = flags;
        }
        fdInfo_[fd].activeTime_ = curtime;
        return true;
    }
    size_t                  fdSize_;
    int                     epollFd_;
    int                     maxSz_;
    U32                     fdTimeoutS_;     //s,fd的超时时间
    int                     epollTimeoutMs_; //ms,epoll的阻塞时间
    CFdMap<__FdInfo>        fdInfo_;         //每个fd的辅助信息
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

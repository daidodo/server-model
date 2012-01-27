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

struct CEpollEvent
{
    typedef struct epoll_event __Event;
    //functions
    explicit CEpollEvent(const __Event & ev):ev_(ev){}
    int Fd() const{return ev_.data.fd;}
    bool Invalid() const{return ev_.data.fd < 0;}
    bool CanInput() const{return ev_.events & EPOLLIN;}
    bool CanOutput() const{return ev_.events & EPOLLOUT;}
    bool Error() const{return (ev_.events & EPOLLERR) || (ev_.events & EPOLLHUP);}
    std::string ToString() const;
private:
    //members
    const __Event & ev_;
};

class CEpoll
{
    typedef CEpollEvent::__Event __Event;
public:
    static std::string EventsName(U32 events);
    CEpoll();
    ~CEpoll(){Destroy();}
    bool IsValid() const{return epollFd_ >= 0;}
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
    //获取fd对应的flags
    U32 GetFlags(int fd) const{return fdInfo_[fd];}
    //修改fd对应的flags
    //如果fd没有flags，则新增
    //如果fd已有flags，则覆盖
    bool ModifyFlags(int fd, U32 flags){
        U32 & oldFlags = fdInfo_[fd];
        if(oldFlags)
            return modifyFdFlags(fd, flags);
        return addFdFlags(fd, flags);
    }
    //将fd从Epoll中移除
    //del: 是否需要从epoll里删除fd
    bool RemoveFd(int fd, bool del = false){
        if(del && epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, 0) < 0)
            return false;
        U32 & flags = fdInfo_[fd];
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
        if(fdSize_){
            int n = epoll_wait(epollFd_, &revents_[0], revents_.size(), epollTimeoutMs_);
            if(n < 0)
                return false;
            revents_.resize(n);
        }else
            usleep(std::min(epollTimeoutMs_ * 1000, 500000));
        return true;
    }
    //根据下标获取读写事件
    //index: 范围在[0, CEpoll::Size())之间
    CEpollEvent operator [](size_t index){
        return CEpollEvent(revents_[index]);
    }
private:
    //将fd加入到epoll中
    //flags将被加上EPOLLET
    bool addFdFlags(int fd, U32 flags);
    //改变fd的flags
    //flags将被加上EPOLLET，并覆盖原flags
    bool modifyFdFlags(int fd, U32 flags);
    //members
    size_t                  fdSize_;
    int                     epollFd_;
    int                     maxSz_;
    int                     epollTimeoutMs_; //ms,epoll的阻塞时间
    CFdMap<U32>             fdInfo_;         //每个fd的flags
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

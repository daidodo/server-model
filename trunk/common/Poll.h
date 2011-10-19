#ifndef DOZERG_POLL_H_20080506
#define DOZERG_POLL_H_20080506

#include <sys/poll.h>
#include <vector>
#include <map>
#include <common/impl/Config.h>

/*
    poll操作封装
        CPoll
//*/

NS_SERVER_BEGIN

class CPoll
{
    typedef struct pollfd           __PollFd;
    typedef __DZ_MAP(int,size_t)    __FdMap;
    class __PollFdHelper{
        const __PollFd & p_;
        U32              expireTime_;   //s,fd的过期时间
    public:
        __PollFdHelper(const __PollFd & p,U32 tt)
            : p_(p)
            , expireTime_(tt)
        {}
        bool Invalid() const{return p_.fd < 0;}
        bool Readable() const{return p_.revents & POLLRDNORM;}
        bool Writable() const{return p_.revents & POLLWRNORM;}
        bool IsExpired(U32 curtime) const{
            return expireTime_ < curtime;
        }
        bool Error() const{
            return p_.revents & POLLERR ||
                p_.revents & POLLNVAL ||
                p_.revents & POLLHUP;
        }
        int FD() const{return p_.fd;}
        U32 ExpireTime() const{return expireTime_;}
        int Flags() const{return p_.events;}
    };
public:
    CPoll():fdtimeout_(0),polltimeout_(1000){}
    //如果fdtimeout_设置为0，表示所有fd永不超时
    void SetFdTimeout(U32 s){fdtimeout_ = s;}
    U32 GetFdTimeout() const{return fdtimeout_;}
    void SetPollTimeout(int ms){polltimeout_ = ms;}
    int GetPollTimeout() const{return polltimeout_;}
    bool CanTimeout() const{return fdtimeout_;}
    size_t Size() const{return pollFd_.size();}
    void Reserve(size_t sz){    //预留空间
        pollFd_.reserve(sz);
        fdTime_.reserve(sz);
    }
    //如果curtime设置为0，表示当前fd永不超时
    void AddFd(int fd,int events,U32 curtime){
        if(fd >= 0){
            size_t & i = fdMap_[fd];
            if(!i){
                pollFd_.push_back(__PollFd());
                __PollFd & pollfd = pollFd_.back();
                pollfd.fd = fd;
                pollfd.events = events;
                pollfd.revents = 0;
                fdTime_.push_back(curtime);
                i = pollFd_.size();
            }else{
                __PollFd & pollfd = pollFd_[i - 1];
                pollfd.fd = fd;
                pollfd.events |= events;
                pollfd.revents = 0;
                fdTime_[i - 1] = curtime;
            }
        }
    }
    void ModifyIndex(size_t index,int events,U32 curtime){
        if(index < pollFd_.size()){
            pollFd_[index].events = events;
            pollFd_[index].revents = 0;
            fdTime_[index] = curtime;
        }
    }
    void RemoveIndex(size_t index){
        int fd = removeIndex(index);
        if(fd >= 0)
            fdMap_.erase(fd);
    }
    void RemoveFd(int fd){
        if(fd >= 0){
            __FdMap::iterator i = fdMap_.find(fd);
            if(i != fdMap_.end()){
                removeIndex(i->second - 1);
                fdMap_.erase(i);
            }
        }
    }
    template<class ForwardIter>
    void RemoveFd(ForwardIter first,ForwardIter last){
        for(;first != last;++first)
            RemoveFd(*first);
    }
    bool Wait(){
        if(!pollFd_.empty())
            return poll(&pollFd_[0],pollFd_.size(),polltimeout_) >= 0;
        usleep(std::min(polltimeout_ * 1000,5000000));
        return true;
    }
    __PollFdHelper operator [](size_t index) const{
        return __PollFdHelper(pollFd_[index],
            (fdtimeout_ && fdTime_[index] ? fdtimeout_ + fdTime_[index] : U32(-1)));
    }
private:
    int removeIndex(size_t index){
        int ret = -1;
        if(index < pollFd_.size()){
            ret = pollFd_[index].fd;
            if(index != pollFd_.size() - 1){
                pollFd_[index] = pollFd_.back();
                fdTime_[index] = fdTime_.back();
            }
            pollFd_.pop_back();
            fdTime_.pop_back();
        }
        return ret;
    }

    __DZ_VECTOR(__PollFd)   pollFd_;
    __DZ_VECTOR(U32)        fdTime_;        //s,fd的加入时间
    __FdMap                 fdMap_;         //fd -> (index + 1)
    U32                     fdtimeout_;     //s,fd的超时时间
    int                     polltimeout_;   //ms,poll的阻塞时间
};

NS_SERVER_END

#endif

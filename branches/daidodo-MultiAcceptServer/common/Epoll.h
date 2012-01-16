#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <sys/epoll.h>
#include <vector>
#include <set>
#include <cstring>   //memset
#include <FdMap.h>   //CFdMap

/*
    epoll������װ,����ETģʽ
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
    //����/��ȡfd�ĳ�ʱʱ��(��)
    //���fdtimeout_����Ϊ0����ʾ����fd������ʱ
    void FdTimeout(U32 s){fdtimeout_ = s;}
    U32 FdTimeout() const{return fdtimeout_;}
    bool CanTimeout() const{return fdtimeout_ != 0;}
    //����/��ȡepoll wait��ʱ��(����)
    void EpollTimeout(int ms){epollTimeout_ = ms;}
    int EpollTimeout() const{return epollTimeout_;}
    //��ȡfd����������
    size_t MaxFdSize() const{return maxsz_;}
    //��ȡ�Ѽ���epoll��fd�ĸ���
    size_t FdSize() const{return fdSize_;}
    //��ȡ��ǰ�ж�д�¼���fd�ĸ���
    size_t Size() const{return revents_.size();}
    //����epoll
    //max_size: fd��������
    bool Create(int max_size){
        if(!IsValid())
            epollfd_ = epoll_create(max_size);
        return IsValid();
    }
    //����epoll
    void Destroy(){
        if(IsValid()){
            close(epollfd_);
            epollfd_ = -1;
        }
    }
    //��ӻ����޸�fd��flags
    //���fdû�У������
    //���fd���У����޸ģ�flags��OR�Ĺ�ϵ
    //flags��������EPOLLET
    bool AddOrModifyFd(int fd, U32 flags, U32 curtime){
        U32 oldFlags = fdInfo_[fd].flags_;
        if(oldFlags == flags)
            return true;
        if(oldFlags)
            return ModifyFd(fd, oldFlags | flags);
        return AddFd(fd, flags, curtime);
    }
    //��fd��fdInfo_���Ƴ�
    //del:�Ƿ���Ҫ��epoll��ɾ��fd
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
    //�ȴ�epoll�¼�
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
    //�����±��ȡ��д�¼�
    //index: ��Χ��[0, CEpoll::Size())֮��
    CEpollEvent operator [](size_t index) const{
        return CEpollEvent(revents_[index], fdInfo_[revents_[index].data.fd].fdtime_, fdtimeout_);
    }
    //��ȡfd�Ƿ��Ѿ���ʱ
    //bool IsFdTimeout(int fd,U32 curtime) const{
    //    return isTimeout(fdInfo_[fd].fdtime_,fdtimeout_,curtime);
    //}
    //��ȡfd�ĳ�ʱʱ���
    //U32 ExpireTime(int fd) const{
    //    return expireTime(fdInfo_[fd].fdtime_,fdtimeout_);
    //}
    //������ȡ���м���epoll��fd
    //const_iterator begin() const{return fdSet_.begin();}
    //const_iterator end() const{return fdSet_.end();}
private:
    //��fd���뵽epoll��
    //flags��������EPOLLET
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
    //�ı�fd��flags
    //flags��������EPOLLET��������ԭflags
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
    U32                     fdtimeout_;     //s,fd�ĳ�ʱʱ��
    int                     epollTimeout_;  //ms,epoll������ʱ��
    CFdMap<__FdInfo>        fdInfo_;        //ÿ��fd�ĸ�����Ϣ
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

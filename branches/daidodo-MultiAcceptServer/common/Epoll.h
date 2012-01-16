#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <sys/epoll.h>
#include <vector>
#include <set>
#include <cstring>   //memset
#include <FdMap.h>   //CFdMap
#include <Tools.h>   //CFdMap

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
    //����/��ȡfd�ĳ�ʱʱ��(��)
    //���fdTimeoutS_����Ϊ0����ʾ����fd������ʱ
    void FdTimeout(U32 s){fdTimeoutS_ = s;}
    U32 FdTimeout() const{return fdTimeoutS_;}
    bool CanTimeout() const{return fdTimeoutS_ != 0;}
    //����/��ȡepoll wait��ʱ��(����)
    void EpollTimeout(int ms){epollTimeoutMs_ = ms;}
    int EpollTimeout() const{return epollTimeoutMs_;}
    //��ȡfd����������
    size_t MaxFdSize() const{return maxSz_;}
    //��ȡ�Ѽ���epoll��fd�ĸ���
    size_t FdSize() const{return fdSize_;}
    //��ȡ��ǰ�ж�д�¼���fd�ĸ���
    size_t Size() const{return revents_.size();}
    //����epoll
    //max_size: fd��������
    bool Create(int max_size){
        if(!IsValid())
            epollFd_ = epoll_create(max_size);
        return IsValid();
    }
    //����epoll
    void Destroy(){
        if(IsValid()){
            close(epollFd_);
            epollFd_ = -1;
        }
    }
    //����fd��Ӧ��flags
    //���fdû��flags��������
    //���fd����flags����OR
    bool AddFlags(int fd, U32 flags, U32 curtime)
    {
        U32 & oldFlags = fdInfo_[fd].flags_;
        if(oldFlags)
            return modifyFdFlags(fd, flags, curtime);
        return addFdFlags(fd, flags, curtime);
    }
    //�޸�fd��Ӧ��flags
    //���fdû��flags��������
    //���fd����flags���򸲸�
    bool ModFlags(int fd, U32 flags, U32 curtime)
    {
        U32 & oldFlags = fdInfo_[fd].flags_;
        if(oldFlags)
            return modifyFdFlags(fd, flags | oldFlags, curtime);
        return addFdFlags(fd, flags, curtime);
    }
    //��fd��Epoll���Ƴ�
    //del: �Ƿ���Ҫ��epoll��ɾ��fd
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
    //�ȴ�epoll�¼�
    //return: true-�ɹ�; false-����
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
    //�����±��ȡ��д�¼�
    //index: ��Χ��[0, CEpoll::Size())֮��
    CEpollEvent operator [](size_t index) const{
        return CEpollEvent(revents_[index], fdInfo_[revents_[index].data.fd].fdtime_, fdTimeoutS_);
    }
    //��ȡfd�Ƿ��Ѿ���ʱ
    //bool IsFdTimeout(int fd,U32 curtime) const{
    //    return isTimeout(fdInfo_[fd].fdtime_,fdTimeoutS_,curtime);
    //}
    //��ȡfd�ĳ�ʱʱ���
    //U32 ExpireTime(int fd) const{
    //    return expireTime(fdInfo_[fd].fdtime_,fdTimeoutS_);
    //}
    //������ȡ���м���epoll��fd
    //const_iterator begin() const{return fdSet_.begin();}
    //const_iterator end() const{return fdSet_.end();}
private:
    //��fd���뵽epoll��
    //flags��������EPOLLET
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
    //�ı�fd��flags
    //flags��������EPOLLET��������ԭflags
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
    U32                     fdTimeoutS_;     //s,fd�ĳ�ʱʱ��
    int                     epollTimeoutMs_; //ms,epoll������ʱ��
    CFdMap<__FdInfo>        fdInfo_;         //ÿ��fd�ĸ�����Ϣ
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

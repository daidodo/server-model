#ifndef DOZERG_EPOLL_H_20080507
#define DOZERG_EPOLL_H_20080507

#include <common/impl/Config.h>
#include <sys/epoll.h>
#include <cstring>          //memset
#include <vector>
#include <set>
#include <common/FdMap.h>   //CFdMap

/*
    epoll������װ,����ETģʽ
        CEpoll
//*/

NS_SERVER_BEGIN

class CEpoll
{
    typedef struct epoll_event  __Event;
    typedef std::set<int>       __FdSet;
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
        , epollTimeout_(4000)
    {}
    ~CEpoll(){Destroy();}
    bool IsValid() const{return epollfd_ >= 0;}
    //����/��ȡfd�ĳ�ʱʱ��(��)
    //���fdtimeout_����Ϊ0����ʾ����fd������ʱ
    void SetFdTimeout(U32 s){fdtimeout_ = s;}
    U32 GetFdTimeout() const{return fdtimeout_;}
    bool CanTimeout() const{return fdtimeout_;}
    //����/��ȡepoll wait��ʱ��(΢��)
    void SetEpollTimeout(int us){epollTimeout_ = us;}
    int GetEpollTimeout() const{return epollTimeout_;}
    //��ȡfd����������
    size_t MaxFdSize() const{return maxsz_;}
    //��ȡ�Ѽ���epoll��fd�ĸ���
    size_t FdSize() const{return fdSet_.size();}
    //��ȡ��ǰ�ж�д�¼���fd�ĸ���
    size_t Size() const{return revents_.size();}
    //����epoll
    //max_size:fd��������
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
    //��fd���뵽epoll��
    //flags��������EPOLLET
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
    //��fd��fdSet_���Ƴ�
    //del:�Ƿ���Ҫ��epoll��ɾ��fd
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
    //�ı�fd�Ķ�дflags
    //flags��������EPOLLET
    bool ModifyFd(int fd,int flags){
        __Event ev;
        memset(&ev,0,sizeof ev);
        ev.events = flags | EPOLLET;
        ev.data.fd =fd;
        return (epoll_ctl(epollfd_,EPOLL_CTL_MOD,fd,&ev) == 0);
    }
    //�ȴ�epoll�¼�
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
    //�����±��ȡ��д�¼�
    //index:��Χ��[0, CEpoll::Size())֮��
    __EventHelper operator [](size_t index){
        return __EventHelper(revents_[index],fdTime_[revents_[index].data.fd],fdtimeout_);
    }
    //��ȡfd�Ƿ��Ѿ���ʱ
    bool IsFdTimeout(int fd,U32 curtime) const{
        return isTimeout(fdTime_[fd],fdtimeout_,curtime);
    }
    //��ȡfd�ĳ�ʱʱ���
    U32 ExpireTime(int fd) const{
        return expireTime(fdTime_[fd],fdtimeout_);
    }
    //������ȡ���м���epoll��fd
    const_iterator begin() const{return fdSet_.begin();}
    const_iterator end() const{return fdSet_.end();}
private:
    int                     epollfd_;
    int                     maxsz_;
    std::vector<__Event>    revents_;
    CFdMap<U32>             fdTime_;        //s,ÿ��fd���ϴλ�Ծʱ��
    __FdSet                 fdSet_;         //��ǰ�����fd����
    U32                     fdtimeout_;     //s,fd�ĳ�ʱʱ��
    int                     epollTimeout_;  //ms,epoll������ʱ��
};

NS_SERVER_END

#endif

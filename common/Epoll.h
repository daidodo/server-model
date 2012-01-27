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
    //��ȡfd��Ӧ��flags
    U32 GetFlags(int fd) const{return fdInfo_[fd];}
    //�޸�fd��Ӧ��flags
    //���fdû��flags��������
    //���fd����flags���򸲸�
    bool ModifyFlags(int fd, U32 flags){
        U32 & oldFlags = fdInfo_[fd];
        if(oldFlags)
            return modifyFdFlags(fd, flags);
        return addFdFlags(fd, flags);
    }
    //��fd��Epoll���Ƴ�
    //del: �Ƿ���Ҫ��epoll��ɾ��fd
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
    //�ȴ�epoll�¼�
    //return: true-�ɹ�; false-����
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
    //�����±��ȡ��д�¼�
    //index: ��Χ��[0, CEpoll::Size())֮��
    CEpollEvent operator [](size_t index){
        return CEpollEvent(revents_[index]);
    }
private:
    //��fd���뵽epoll��
    //flags��������EPOLLET
    bool addFdFlags(int fd, U32 flags);
    //�ı�fd��flags
    //flags��������EPOLLET��������ԭflags
    bool modifyFdFlags(int fd, U32 flags);
    //members
    size_t                  fdSize_;
    int                     epollFd_;
    int                     maxSz_;
    int                     epollTimeoutMs_; //ms,epoll������ʱ��
    CFdMap<U32>             fdInfo_;         //ÿ��fd��flags
    std::vector<__Event>    revents_;
};

NS_SERVER_END

#endif

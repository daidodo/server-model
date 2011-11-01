#ifndef DOZERG_FD_MAP_H_20071228
#define DOZERG_FD_MAP_H_20071228

/*
    �ṩfd�������ӳ��
    ��fd��Ϊ��������Ҫ���б߽�����Զ�����
        CFdMap      �ɼ̳�,�̲߳���ȫ
        CFdSockMap  ��fd��Ϊ������socket������,
                    ����socket����Ĺرպ��ͷ�
                    ���δʹ��CSharedPtr����Ҫ��ģ�����Sockʵ����PutObject��̬����
                    �̰߳�ȫ
    History
        20080604    CFdSockMap���Ӷ�CSharedPtr��֧�֣���Ҫ��putSock�Ĵ���ͬ
        20080910    CFdSockMap����sz_�ֶΣ�Size()�����������Ϊ����fd�ĸ���
                    GetSock()�����ڵ�null_ptr�������ͼ���const�޶�
        20080911    CFdSockMap����CloseSock()����������رմ�������ʱƵ������
                    CFdSockMap����SetSock()���������أ�ͨ��һ�β����ȸ������Ӷ����ֵõ��ɵ����Ӷ���
                    CFdSockMap�޸�ԭSetSock()����������del���������롰�ر����ӡ��롰�ͷŶ��󡱲���
        20080917    ��CFdMap::operator [] const����DEFAULT����
                    CFdMap��CFdSockMap����ģ�����Container�������ڲ���������
                    CFdMap����reserve()����
        20080920    CFdSockMapʹ��ģ���������������
        20081010    CFdSockMap::GetSock()��������ֵ��Ϊ__SockPtr������Ϊ����const���û������������map_[fd]
                    ����CFdSockMap::GetSock(int, __SockPtr &) const���أ�����һ����ʱ����
        20081013    ����CFdSockMap::GetSock(ForwardIter, ForwardIter, OutputIter) const���أ�������ȡ���Ӷ���
//*/

#include <vector>
#include <cassert>
#include <algorithm>
#include <common/Mutex.h>
#include <common/SharedPtr.h>

NS_SERVER_BEGIN

template<class T, class Container = __DZ_VECTOR(T)>
struct CFdMap
{
    typedef Container   container_type;
    typedef T           value_type;
    typedef T &         reference;
    typedef const T &   const_reference;
    explicit CFdMap(size_t sz = 100){map_.reserve(sz);}
    size_t size() const{return map_.size();}
    void reserve(size_t sz){map_.reserve(sz);}
    reference operator [](int fd){
        assert(fd >= 0);
        if(size_t(fd) >= map_.size())
            map_.resize(fd + 1);
        return map_[fd];
    }
    const_reference operator [](int fd) const{
        static const value_type DEFAULT = value_type();
        if(fd >= 0 && size_t(fd) < map_.size())
            return map_[fd];
        return DEFAULT;
    }
protected:
    mutable container_type map_;
};

template<
    class Sock,
    class SockPtr = Sock *,
    class Container = __DZ_VECTOR(SockPtr),
    class LockT = CMutex
>class CFdSockMap
{
    typedef Sock                __Sock;
    typedef SockPtr             __SockPtr;
    typedef Container           __SockMap;
    typedef LockT               lock_type;
    typedef CGuard<lock_type>   guard_type;
    static void putSock(Sock *& p){
        __Sock::PutObject(p);
        p = 0;
    }
    template<bool B, class T>
    static void putSock(CSharedPtr<Sock, B, T> & p){
        if(p){
            p->Close();
            p = 0;
        }
    }
public:
    explicit CFdSockMap(size_t sz = 100)
        : map_(sz)
        , sz_(0)
    {}
    ~CFdSockMap(){
        for(typename __SockMap::iterator it = map_.begin();it != map_.end();++it)
            putSock(*it);
        sz_ = 0;
    }
    //�ر�ԭ���ӣ����ó�������p
    //del��ʾ�Ƿ��ͷ�ԭ���Ӷ���
    //�������CSharedPtr����del��������
    void SetSock(int fd, const __SockPtr & p, bool del = true){
        if(fd >= 0){
            guard_type g(lock_);
            if(fd < int(map_.size())){
                __SockPtr & cur = map_[fd];
                if(cur != p){
                    if(cur){
                        --sz_;
                        if(del){
                            putSock(cur);
                        }else{
                            cur->Close();
                            cur = 0;
                        }
                    }
                    if(p){
                        cur = p;
                        ++sz_;
                    }
                }
            }else if(p){
                map_.resize(fd + 1);
                map_[fd] = p;
                ++sz_;
            }
        }
    }
    //�ر�ԭ���ӣ����ó�������p
    //���ͷ�ԭ���Ӷ��󣬲�ͨ��old����
    void SetSock(int fd, const __SockPtr & p, __SockPtr & old){
        old = 0;
        if(fd >= 0){
            guard_type g(lock_);
            if(fd < int(map_.size())){
                __SockPtr & cur = map_[fd];
                if(cur != p){
                    std::swap(cur, old);
                    if(old){
                        old->Close();
                        --sz_;
                    }
                    if(p){
                        cur = p;
                        ++sz_;
                    }
                }
            }else if(p){
                map_.resize(fd + 1);
                map_[fd] = p;
                ++sz_;
            }
        }
    }
    __SockPtr GetSock(int fd) const{
        if(fd >= 0){
            guard_type g(lock_);
            if(fd < int(map_.size()))
                return map_[fd];
        }
        return 0;
    }
    void GetSock(int fd, __SockPtr & p) const{   //������ذ汾���Ա���һ����ʱ����
        p = 0;
        if(fd >= 0){
            guard_type g(lock_);
            if(fd < int(map_.size()))
                p = map_[fd];
        }
   }
    //������ȡ����
    //�ѷ�Χ[first, last)ָ����fd�����Ӷ���һһ����[dst_first, ...)��Χ��
    //�û�����֤Ŀ�ĵ�dst_first����Ч�Ժͷ�Χ
    template<class ForwardIter, class OutputIter>
    void GetSock(ForwardIter first, ForwardIter last, OutputIter dst_first) const{
        guard_type g(lock_);
        const int sz = int(map_.size());
        for(;first != last;++first, ++dst_first){
            const int & fd = *first;
            *dst_first = (fd >= 0 && fd < sz ? map_[fd] : 0);
        }
    }
    //�����ر�����
    //���δʹ��CSharedPtr�����ͷ����Ӷ���
    //���ʹ��CSharedPtr�����ͷ����Ӷ���
    template<class ForwardIter>
    void CloseSock(ForwardIter first, ForwardIter last){
        guard_type g(lock_);
        const int sz = int(map_.size());
        for(;first != last;++first){
            const int & fd = *first;
            if(fd >= 0 && fd < sz){
                if(map_[fd])
                    --sz_;
                putSock(map_[fd]);
            }
        }
    }
    //�����ر����ӣ����õ����Ӷ���
    //������GetSock��CloseSock�Ĺ����ܺ�
    template<class ForwardIter, class OutputIter>
    void CloseSock(ForwardIter first, ForwardIter last, OutputIter dst_first){
        guard_type g(lock_);
        const int sz = int(map_.size());
        for(;first != last;++first, ++dst_first){
            const int & fd = *first;
            if(fd >= 0 && fd < sz){
                *dst_first = map_[fd];
                if(map_[fd])
                    --sz_;
                putSock(map_[fd]);
            }else
                *dst_first = 0;
        }
    }
    //��ȡ����������
    size_t Size() const{
        guard_type g(lock_);
        return sz_;
    }
private:
    __SockMap   map_;
    size_t      sz_;    //��Ч(fd -> pSock)����
    lock_type   lock_;
};

NS_SERVER_END

#endif

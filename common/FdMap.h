#ifndef DOZERG_FD_MAP_H_20071228
#define DOZERG_FD_MAP_H_20071228

/*
    提供fd到对象的映射
    以fd作为索引，主要进行边界检查和自动伸缩
        CFdMap      可继承,线程不安全
        CFdSockMap  以fd作为索引的socket对象针,
                    负责socket对象的关闭和释放
                    如果未使用CSharedPtr，则要求模板参数Sock实现了PutObject静态函数
                    线程安全
    History
        20080604    CFdSockMap增加对CSharedPtr的支持，主要是putSock的处理不同
        20080910    CFdSockMap增加sz_字段，Size()函数的意义改为在线fd的个数
                    GetSock()函数内的null_ptr对象类型加上const限定
        20080911    CFdSockMap增加CloseSock()函数，避免关闭大量连接时频繁加锁
                    CFdSockMap增加SetSock()函数的重载，通过一次操作既更换连接对象，又得到旧的连接对象
                    CFdSockMap修改原SetSock()函数，增加del参数，分离“关闭连接”与“释放对象”操作
        20080917    给CFdMap::operator [] const增加DEFAULT变量
                    CFdMap和CFdSockMap增加模板参数Container，决定内部容器类型
                    CFdMap增加reserve()函数
        20080920    CFdSockMap使用模板参数决定锁类型
        20081010    CFdSockMap::GetSock()函数返回值改为__SockPtr对象，因为返回const引用会造成无锁访问map_[fd]
                    增加CFdSockMap::GetSock(int, __SockPtr &) const重载，减少一个临时对象
        20081013    增加CFdSockMap::GetSock(ForwardIter, ForwardIter, OutputIter) const重载，批量获取连接对象
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
    //关闭原连接，设置成新连接p
    //del表示是否释放原连接对象
    //如果采用CSharedPtr，则del参数无用
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
    //关闭原连接，设置成新连接p
    //不释放原连接对象，并通过old返回
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
    void GetSock(int fd, __SockPtr & p) const{   //这个重载版本可以避免一个临时对象
        p = 0;
        if(fd >= 0){
            guard_type g(lock_);
            if(fd < int(map_.size()))
                p = map_[fd];
        }
   }
    //批量获取连接
    //把范围[first, last)指定的fd的连接对象一一存入[dst_first, ...)范围里
    //用户负责保证目的地dst_first的有效性和范围
    template<class ForwardIter, class OutputIter>
    void GetSock(ForwardIter first, ForwardIter last, OutputIter dst_first) const{
        guard_type g(lock_);
        const int sz = int(map_.size());
        for(;first != last;++first, ++dst_first){
            const int & fd = *first;
            *dst_first = (fd >= 0 && fd < sz ? map_[fd] : 0);
        }
    }
    //批量关闭连接
    //如果未使用CSharedPtr，会释放连接对象
    //如果使用CSharedPtr，不释放连接对象
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
    //批量关闭连接，并得到连接对象
    //即上面GetSock和CloseSock的功能总和
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
    //获取在线连接数
    size_t Size() const{
        guard_type g(lock_);
        return sz_;
    }
private:
    __SockMap   map_;
    size_t      sz_;    //有效(fd -> pSock)数量
    lock_type   lock_;
};

NS_SERVER_END

#endif

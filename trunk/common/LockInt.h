#ifndef DOZERG_LOCK_INT_H_20070916
#define DOZERG_LOCK_INT_H_20070916

/*
    提供加锁保护的整数类型模板
        CLockInt        提供加锁的整数类型
        CLockIntMax     有统计最大值功能的加锁整数类型
        CLockIntRange   对整数值进行锁定
    History
        20070925    把pthread_mutex_t改成CMutex
        20080509    使用guard_type;加入CLockIntRange
        20080920    使用模板参数决定锁类型
//*/

#include <common/impl/Config.h>
#include <map>
#include <list>
#include <algorithm>    //std::for_each
#include <utility>      //std::make_pair
#include <common/Mutex.h>

NS_SERVER_BEGIN

template<typename T,class LockT = CMutex>
struct CLockInt
{
    typedef T                   int_type;
    typedef LockT               lock_type;
    typedef CGuard<lock_type>   guard_type;
    CLockInt(T c = 0):v_(c){}
    CLockInt(const CLockInt & c):v_(c.operator T()){}
    operator T() const{
        guard_type g(lock_);
        return v_;
    }
    T operator =(T c){
        guard_type g(lock_);
        return (v_ = c);
    }
    T operator =(const CLockInt & c){
        T ret = c.operator T();
        if(&c != this){
            guard_type g(lock_);
            v_ = ret;
        }
        return ret;
    }
    T operator ++(){
        guard_type g(lock_);
        return ++v_;
    }
    T operator --(){
        guard_type g(lock_);
        return --v_;
    }
    T operator ++(int){
        guard_type g(lock_);
        return v_++;
    }
    T operator --(int){
        guard_type g(lock_);
        return v_--;
    }
    T operator +=(T c){
        guard_type g(lock_);
        return v_ += c;
    }
    T operator -=(T c){
        guard_type g(lock_);
        return v_ -= c;
    }
    T Reset(T c = 0){
        guard_type g(lock_);
        T ret = v_;
        v_ = c;
        return ret;
    }
private:
    int_type    v_;
    lock_type   lock_;
};

template<typename T,class LockT = CMutex>
struct CLockIntMax
{
    typedef T                   int_type;
    typedef LockT               lock_type;
    typedef CGuard<lock_type>   guard_type;
    explicit CLockIntMax(T c = 0):v_(c),max_(c){}
    T operator =(T c){
        guard_type g(lock_);
        if((v_ = c) > max_)
            max_ = c;
        return c;
    }
    T operator ++(){
        guard_type g(lock_);
        if(++v_ > max_)
            max_ = v_;
        return v_;
    }
    T operator --(){
        guard_type g(lock_);
        return --v_;
    }
    T operator +=(T c){
        guard_type g(lock_);
        if((v_ += c) > max_)
            max_ = v_;
        return v_;
    }
    T operator -=(T c){
        guard_type g(lock_);
        return v_ -= c;
    }
    T Value() const{
        guard_type g(lock_);
        return v_;
    }
    T Max() const{
        guard_type g(lock_);
        return max_;
    }
    T Reset(T c = 0){
        guard_type g(lock_);
        T ret = v_;
        if((v_ = c) > max_)
            max_ = c;
        return ret;
    }
    T ResetMax(){
        guard_type g(lock_);
        T ret = max_;
        max_ = v_;
        return ret;
    }
private:
    CLockIntMax(CLockIntMax &);     //disable copy and assignment
    CLockIntMax & operator =(CLockIntMax &);
    int_type    v_,max_;
    lock_type lock_;
};

template<typename T,class LockT = CMutex>
struct CLockIntRange
{
    typedef T                   int_type;
    typedef LockT               lock_type;
    typedef CGuard<lock_type>   guard_type;
private:
    typedef std::map<T,lock_type *> __Map;
    typedef std::list<lock_type *>  __List;
    typedef std::allocator<lock_type>   __MAlloc;
    static lock_type * getLock(){
        lock_type * ret = __MAlloc().allocate(1);
        return new (ret) lock_type;
    }
    static void putLock(lock_type *& p) __DZ_NOTHROW{
        Tools::Destroy(p,__MAlloc());
    }
public:
    CLockIntRange(){}
    ~CLockIntRange(){cleanup();}
    void Lock(const T & x){
        guard_type g(lock_);
        get(x,1)->Lock();
    }
    void Unlock(const T & x){
        guard_type g(lock_);
        lock_type * m = get(x,2);
        if(m)
            m->Unlock();
    }
    lock_type & GetLock(const T & x){
        guard_type g(lock_);
        return *get(x,1);
    }
private:
    CLockIntRange(CLockIntRange &);     //disable copy and assignment
    CLockIntRange & operator =(CLockIntRange &);
    void cleanup() __DZ_NOTHROW{
        for(typename __Map::iterator it = map_.begin();it != map_.end();++it)
            putLock(it->second);
        std::for_each(idle_.begin(),idle_.end(),putLock);
    }
    /* op:
        0   查找x
        1   没有x则添加
        2   有x则删除
       返回找到或新添加的lock_type *
    //*/
    lock_type * get(const T & x,int op){
        lock_type * ret = 0;
        typename __Map::iterator wh = map_.find(x);
        if(wh == map_.end()){
            if(op == 1){    //添加
                if(idle_.empty()){
                    ret = getLock();
                }else{
                    ret = idle_.front();
                    idle_.pop_front();
                }
                map_.insert(std::make_pair(x,ret));
            }
        }else{
            ret = wh->second;
            if(op == 2){    //删除
                map_.erase(wh);
                if(ret)
                    idle_.push_back(ret);
            }
        }
        return ret;
    }
    __Map       map_;   //T -> lock_type
    __List      idle_;  //空闲的锁
    lock_type   lock_;
};

NS_SERVER_END

#endif

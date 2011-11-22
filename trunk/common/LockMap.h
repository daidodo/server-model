#ifndef DOZERG_LOCKED_SET_H_20070905
#define DOZERG_LOCKED_SET_H_20070905

/*
    加锁的set和map
        CLockSet
        CLockMap
    History
        20080619    把CMutex改成模板参数，可以支持各种锁
//*/
#include <set>
#include <map>
#include <functional>       //std::less
#include <common/Mutex.h>

NS_SERVER_BEGIN

template<class T, class LockT = CMutex, class Pr = std::less<T> >
class CLockSet : public __DZ_SET1(T, Pr)
{
//typedefs
    typedef __DZ_SET1(T, Pr)                     __MyBase;
public:
    typedef __MyBase                            container_type;
    typedef typename __MyBase::iterator         iterator;
    typedef typename __MyBase::const_iterator   const_iterator;
    typedef LockT                               lock_type;
    typedef CLockAdapter<lock_type>             adapter_type;
    typedef CGuard<lock_type>                   guard_type;
    bool insertL(const T & v){
        guard_type g(lock_);
        return __MyBase::insert(v).second;
    }
    bool findL(const T & v) const{
        guard_type g(lock_);
        typename __MyBase::const_iterator wh = __MyBase::find(v);
        return wh != __MyBase::end();
    }
    size_t eraseL(const T & v){
        guard_type g(lock_);
        return __MyBase::erase(v);
    }
    size_t Size() const{
        guard_type g(lock_);
        return __MyBase::size();
    }
    lock_type & GetLock(){return lock_;}
    void Lock(bool bWrite = true){
        bWrite ? adapter_type().WriteLock(lock_) :
            adapter_type().ReadLock(lock_);
    }
    void Unlock(){
        adapter_type().Unlock(lock_);
    }
//fields
private:
    lock_type lock_;
};

template<class Key, class Value, class LockT = CMutex, class Pr = std::less<Key> >
class CLockMap:public __DZ_MAP1(Key, Value, Pr)
{
//typedefs
    typedef __DZ_MAP1(Key, Value, Pr)             __MyBase;
public:
    typedef __MyBase                            container_type;
    typedef typename __MyBase::iterator         iterator;
    typedef typename __MyBase::const_iterator   const_iterator;
    typedef typename __MyBase::value_type       value_type;
    typedef typename __MyBase::key_type         key_type;
    typedef typename __MyBase::mapped_type      mapped_type;
    typedef LockT                               lock_type;
    typedef CLockAdapter<lock_type>             adapter_type;
    typedef CGuard<lock_type>                   guard_type;
    bool insertL(const value_type & v){
        guard_type g(lock_);
        return __MyBase::insert(v).second;
    }
    bool insertL(const key_type & v, const mapped_type & d = mapped_type()){
        guard_type g(lock_);
        return __MyBase::insert(value_type(v, d)).second;
    }
    size_t eraseL(const key_type & v){
        guard_type g(lock_);
        return __MyBase::erase(v);
    }
    bool pickL(const key_type & v, mapped_type & p){    //搜索并删除
        guard_type g(lock_);
        iterator wh = find(v);
        if(wh != __MyBase::end()){
            p = wh->second;
            erase(wh);
            return true;
        }
        return false;
    }
    size_t Size() const{
        guard_type g(lock_);
        return __MyBase::size();
    }
    lock_type & GetLock(){return lock_;}
    void Lock(bool bWrite = true){
        bWrite ? adapter_type().WriteLock(lock_) :
            adapter_type().ReadLock(lock_);
    }
    void Unlock(){
        adapter_type().Unlock(lock_);
    }
//fields
private:
    lock_type lock_;
};

NS_SERVER_END

#endif

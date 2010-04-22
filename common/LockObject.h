#ifndef DOZERG_LOCK_OBJECT_H_20080103
#define DOZERG_LOCK_OBJECT_H_20080103

/*
    对象的加锁访问,适合本身无锁,但需要互斥访问的数据
        CLockObject     提供加锁的访问
        CRWLockObject   提供加锁的读和写
        CLockContainer  提供对一般容器的简单加锁机制,在必要的时候应该专门定义相应的加锁容器
    History
        20080920    用模板参数决定锁类型
                    合并CRWLockObject和CLockObject
        20081006    去掉CLockObject多余的bWrite模板参数
//*/

#include <common/impl/LockObj_impl.h>

NS_SERVER_BEGIN

template<class T,class LockT>
class CLockObject
{
    typedef __lockPtr<T,LockT,true>         __Writable;
    typedef __lockPtr<const T,LockT,false>  __Readable;
    typedef __Writable                      __Locked;
public:
    typedef __Locked::lock_type     lock_type;
    typedef __Locked::guard_type    guard_type;
    typedef CLockAdapter<lock_type> adapter_type;
    typedef T                       value_type;
    typedef T *                     pointer;
    typedef T &                     reference;
    typedef const T *               const_pointer;
    typedef const T &               const_reference;
    explicit CLockObject(T & r):ref_(r){}
    lock_type & GetLock() const{return lock_;}
    void Lock(bool bWrite = false){
        bWrite ? adapter_type().WriteLock(lock_) : adapter_type().ReadLock(lock_);
    }
    void Unlock(){
        adapter_type().Unlock(lock_);
    }
    //加锁的访问方式
    __Locked LockPointer(){
        return __Locked(&ref_,lock_);
    }
    __Writable WritePointer(){
        return __Writable(&ref_,lock_);
    }
    __Readable ReadPointer() const{
        return __Readable(&ref_,lock_);
    }
    //无锁的访问方式
    pointer operator ->(){return &ref_;}
    reference operator *(){return ref_;}
    const_pointer operator ->() const{return &ref_;}
    const_reference operator *() const{return ref_;}
private:
    reference   ref_;
    CMutex      lock_;
};

template<class Container,class LockT>
class CLockContainer : public Container
{
    typedef __lockPtr<Container,LockT,true>         __Writable;
    typedef __lockPtr<const Container,LockT,false>  __Readable;
    typedef __Writable                              __Locked;
public:
    typedef __Locked::lock_type     lock_type;
    typedef __Locked::guard_type    guard_type;
    typedef CLockAdapter<lock_type> adapter_type;
    typedef Container               container_type;
    lock_type & GetLock() const{return lock_;}
    void Lock(bool bWrite = false){
        bWrite ? adapter_type().WriteLock(lock_) : adapter_type().ReadLock(lock_);
    }
    void Unlock(){
        adapter_type().Unlock(lock_);
    }
    //加锁的访问方式
    __Locked LockPointer(){
        return __Locked(this,lock_);
    }
    __Writable WritePointer(){
        return __Writable(this,lock_);
    }
    __Readable ReadPointer() const{
        return __Readable(this,lock_);
    }
private:
    lock_type lock_;
};

NS_SERVER_END

#endif

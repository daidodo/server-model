#ifndef DOZERG_LOCK_IMPL_H_20080226
#define DOZERG_LOCK_IMPL_H_20080226

/*
    CLockObject等的内部实现
        __lockPtr
    History:
        20080920    使用模板参数决定锁类型
                    合并__rwLockPointer和__lockPtr
//*/

#include <common/Mutex.h>

NS_IMPL_BEGIN

template<class T, class LockT, bool bWrite>
struct __lockPtr{
    typedef LockT               lock_type;
    typedef CGuard<lock_type>   guard_type;
private:
    typedef T *                 pointer;
    typedef T &                 reference;
    const pointer   p_;
    guard_type      g_;
public:
    __lockPtr(pointer p, lock_type & lock):p_(p), g_(lock, bWrite){}
    pointer operator ->() const{return p_;}
    reference operator *() const{return *p_;}
};

NS_IMPL_END

#endif

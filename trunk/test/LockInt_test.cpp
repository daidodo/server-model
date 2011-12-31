#include "comm.h"

#include <common/LockInt.h>

template<class Int>
static bool testLockIntRange()
{
    typedef CLockIntRange<Int>  __LockRange;
    __LockRange intrange;
    for(Int i = 0;i < 100;++i)
        intrange.Lock(i);
    typename __LockRange::lock_type & lock = intrange.GetLock(50);
    if(lock.TryLock()){
        cerr<<"lock.TryLock() would fail but succ\n";
        return false;
    }
    for(Int i = 100;i > 0;--i)
        intrange.Unlock(i -  1);
    if(!lock.TryLock()){
        cerr<<"lock.TryLock() failed\n";
        return false;
    }
    lock.Unlock();
    return true;
}

int main()
{
    if(!testLockIntRange<int>())
        return 1;
    if(!testLockIntRange<CLockInt<int> >())
        return 1;
    cout<<"LockInt test succ\n";
    return 0;
}

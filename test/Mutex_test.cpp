#include "comm.h"

#include <common/Mutex.h>

template<class LockT>
static bool testMutexOp(LockT & m, const char * lockName)
{
    assert(lockName);
    m.Lock();
    if(m.TryLock()){
        cerr<<lockName<<"TryLock() should fail but succ\n";
        return false;
    }
    m.Unlock();
    if(!m.TimeLock(1000)){
        cerr<<lockName<<"TimeLock(1000) failed\n";
        return false;
    }
    m.Unlock();
    return true;
}

static bool testMutex()
{
    CMutex m;
    if(!testMutexOp(m, "CMutex - ")){
        cerr<<"testMutexOp<CMutex>() failed\n";
        return false;
    }
    CAttrMutex am(true);
    if(!testMutexOp(m, "CAttrMutex - ")){
        cerr<<"testMutexOp<CAttrMutex>() failed\n";
        return false;
    }
    return true;
}

static bool testRWLock()
{
    CRWLock m;
    //read lock
    m.ReadLock();
    m.ReadLock();
    if(!m.TryReadLock()){
        cerr<<"CRWLock - TryReadLock() failed\n";
        return false;
    }
    if(!m.TimeReadLock(1000)){
        cerr<<"CRWLock - TimeReadLock(1000) failed\n";
        return false;
    }
    m.Unlock();
    m.Unlock();
    m.Unlock();
    m.Unlock();
    //write lock
    m.WriteLock();
    if(m.TryWriteLock()){
        cerr<<"CRWLock - TryWriteLock() should fail but succ\n";
        return false;
    }
    m.Unlock();
    if(!m.TimeWriteLock(1000)){
        cerr<<"CRWLock - TimeWriteLock(1000) failed\n";
        return false;
    }
    m.Unlock();
    return true;
}

static bool testSpinLock()
{
    CSpinLock m(true);
    m.Lock();
    if(m.TryLock()){
        cerr<<"CSpinLock - TryLock() should fail but succ\n";
        return false;
    }
    m.Unlock();
    return true;
}

template<class T>
static bool testLockAdapterOp()
{
    CLockAdapter<T> ad;
    typename CLockAdapter<T>::lock_type m;
    ad.ReadLock(m);
    ad.ReadToWrite(m);
    ad.Unlock(m);
    ad.WriteLock(m);
    ad.WriteToRead(m);
    ad.Unlock(m);
    return true;
}

static bool testLockAdapter()
{
    if(!testLockAdapterOp<CMutex>())
        return false;
    if(!testLockAdapterOp<CAttrMutex>())
        return false;
    if(!testLockAdapterOp<CRWLock>())
        return false;
    if(!testLockAdapterOp<CSpinLock>())
        return false;
    return true;
}

template<class T>
static bool testGuardOp()
{
    typedef CGuard<T> guard_type;
    typedef typename guard_type::lock_type lock_type;
    lock_type m;
    {guard_type g(m, true);}
    {guard_type g(m, false);}
    {guard_type g(&m, true);}
    {guard_type g(&m, false);}
    const lock_type & n = m;
    {guard_type g(n);}
    {guard_type g(&n);}
    return true;
}

static bool testGuard()
{
    if(!testGuardOp<CMutex>())
        return false;
    if(!testGuardOp<CAttrMutex>())
        return false;

    return true;
}

static void OnceFun()
{
    static int c = 0;
    if(++c > 1){
        cerr<<"OnceFun - c="<<c<<" > 1\n";
        exit(1);
    }
}

static bool testOnceGuard()
{
    COnceGuard g(OnceFun);
    g.RunOnce();
    return true;
}

int main()
{
    if(!testMutex())
        return 1;
    if(!testRWLock())
        return 1;
    if(!testSpinLock())
        return 1;
    if(!testLockAdapter())
        return 1;
    if(!testGuard())
        return 1;
    if(!testOnceGuard())
        return 1;
    cout<<"Mutex test succ\n";
    return 0;
}

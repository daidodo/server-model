#ifndef DOZERG_MUTEX_H_20070924
#define DOZERG_MUTEX_H_20070924

/*
    对POSIX锁机制进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CMutex          互斥锁
        CAttrMutex      带属性的互斥锁
        CCondition      条件变量
        CRWLock         读写锁
        CSpinLock       自旋锁
        CLockAdapter    锁适配器
        CGuard          锁守卫
        COnceGuard      确保某些函数只运行一次
    History:
        20080520    加入COnceGuard
        20080916    CMutex::Lock()和CMutex::Unlock()改为void返回值
                    在DEBUG模式下使CMutex能够检测死锁，并抛出异常
        20080920    加入CSpinLock
                    统一所有的guard类型为CGuard，用模板参数决定锁类型
                    加入CLockAdapter，统一锁的行为
        20081010    CGuard构造函数中bWrite的默认值修改为true
                    修改各种锁成员函数的const属性
                    在CGuard构造函数中根据锁的const属性，自动选择加锁函数
//*/

#include <pthread.h>
#include <stdexcept>        //std::runtime_error
#include <common/Tools.h>   //Tools::GetTimespec, Tools::ErrorMsg

NS_SERVER_BEGIN

class CCondition;

//lock types
class CMutex
{
    friend class CCondition;
    CMutex(const CMutex &);
    CMutex & operator =(const CMutex &);
protected:
    mutable pthread_mutex_t mutex_;
    explicit CMutex(int){}  //给子类重新初始化mutex_的机会
public:
    CMutex(){
#ifdef NDEBUG
        pthread_mutex_init(&mutex_, 0);
#else   //check dead lock
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(&mutex_, &attr);
        pthread_mutexattr_destroy(&attr);
#endif
    }
    virtual ~CMutex(){
        pthread_mutex_destroy(&mutex_);
    }
    void Lock() throw(std::runtime_error){
        int eno = pthread_mutex_lock(&mutex_);
        if(eno)
            throw std::runtime_error(Tools::ErrorMsg(eno).c_str());
    }
    bool TryLock(){
        return !pthread_mutex_trylock(&mutex_);
    }
    void Unlock() const throw(std::runtime_error){
        int eno = pthread_mutex_unlock(&mutex_);
        if(eno)
            throw std::runtime_error(Tools::ErrorMsg(eno).c_str());
    }
    //在指定的timeMs毫秒内如果不能lock,返回false
    bool TimeLock(U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return !pthread_mutex_timedlock(&mutex_, &ts);
    }
};

class CAttrMutex : public CMutex
{
protected:
    bool    pshared_;   //是否进程间共享
    int     type_;
public:
    explicit CAttrMutex(bool pshared = false, int type = PTHREAD_MUTEX_DEFAULT)
        : CMutex(0)
        , pshared_(pshared)
        , type_(type)
    {
        pthread_mutexattr_t attr;
        if(initAttr(attr)){
            pthread_mutex_init(&mutex_, &attr);
            pthread_mutexattr_destroy(&attr);
        }else{
            pshared_ = false;
            type_ = PTHREAD_MUTEX_DEFAULT;
            pthread_mutex_init(&mutex_, 0);
        }
    }
    bool ProcessShared() const{return pshared_;}
    int Type() const{return type_;}
private:
    bool initAttr(pthread_mutexattr_t & attr){
        if(pthread_mutexattr_init(&attr))
            return false;
        if(!pthread_mutexattr_setpshared(&attr, (pshared_ ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE)) &&
            !pthread_mutexattr_settype(&attr, type_))
            return true;
        pthread_mutexattr_destroy(&attr);
        return false;
    }
};

class CCondition
{
    CCondition(const CCondition &);
    CCondition & operator =(const CCondition &);
protected:
    mutable pthread_cond_t cond_;
public:
    CCondition(){
        pthread_cond_init(&cond_, 0);
    }
    virtual ~CCondition(){
        pthread_cond_destroy(&cond_);
    }
//以下函数成功返回true;失败返回false
    bool Signal(){
        return !pthread_cond_signal(&cond_);
    }
    bool Broadcast(){
        return !pthread_cond_broadcast(&cond_);
    }
    bool Wait(CMutex & m){
        return !pthread_cond_wait(&cond_, &m.mutex_);
    }
    //等待指定的timeMs毫秒
    bool TimeWait(CMutex & m, U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return !pthread_cond_timedwait(&cond_, &m.mutex_, &ts);
    }
};

class CRWLock
{
    CRWLock(const CRWLock &);
    CRWLock & operator =(const CRWLock &);
protected:
    mutable pthread_rwlock_t lock_;
public:
    CRWLock(){
        pthread_rwlock_init(&lock_, 0);
    }
    ~CRWLock(){
        pthread_rwlock_destroy(&lock_);
    }
    void ReadLock() const throw(std::runtime_error){
        int eno = pthread_rwlock_rdlock(&lock_);
        if(eno)
            throw std::runtime_error(Tools::ErrorMsg(eno).c_str());
    }
    bool TryReadLock() const{
        return !pthread_rwlock_tryrdlock(&lock_);
    }
    //在指定的timeMs毫秒内如果不能rdlock,返回false
    bool TimeReadLock(U32 timeMs) const{
        timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return !pthread_rwlock_timedrdlock(&lock_, &ts);
    }
    void WriteLock() throw(std::runtime_error){
        int eno = pthread_rwlock_wrlock(&lock_);
        if(eno)
            throw std::runtime_error(Tools::ErrorMsg(eno).c_str());
    }
    bool TryWriteLock(){
        return !pthread_rwlock_trywrlock(&lock_);
    }
    //在指定的timeMs毫秒内如果不能wrlock,返回false
    bool TimeWriteLock(U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return !pthread_rwlock_timedwrlock(&lock_, &ts);
    }
    void Unlock() const{
        pthread_rwlock_unlock(&lock_);
    }
};

#if defined(_POSIX_SPIN_LOCKS) || defined(__DISPLAY_CODE)
class CSpinLock
{
    CSpinLock(const CSpinLock &);
    CSpinLock & operator =(const CSpinLock &);
protected:
    mutable pthread_spinlock_t lock_;
public:
    explicit CSpinLock(int pshared = false){
        int ps = (pshared ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE);
        pthread_spin_init(&lock_, ps);
    }
    ~CSpinLock(){
        pthread_spin_destroy(&lock_);
    }
    void Lock() throw(std::runtime_error){
        int eno = pthread_spin_lock(&lock_);
        if(eno)
            throw std::runtime_error(Tools::ErrorMsg(eno).c_str());
    }
    bool TryLock(){
        return !pthread_spin_trylock(&lock_);
    }
    void Unlock() const{
        pthread_spin_unlock(&lock_);
    }
};
#else
struct CSpinLock : public CMutex
{
    explicit CSpinLock(int pshared = false){}
};
#endif

//adapters for lock types
template<class T>struct CLockAdapter{};

template<>struct CLockAdapter<CMutex>
{
    typedef CMutex      lock_type;
    void Unlock(const lock_type & m) const{m.Unlock();}
    void ReadLock(const lock_type & m) const{const_cast<lock_type &>(m).Lock();}
    void WriteLock(lock_type & m) const{m.Lock();}
    void ReadToWrite(lock_type &) const{}
    void WriteToRead(const lock_type &) const{}
};

template<>struct CLockAdapter<CAttrMutex>
{
    typedef CAttrMutex  lock_type;
    void Unlock(const lock_type & m) const{m.Unlock();}
    void ReadLock(const lock_type & m) const{const_cast<lock_type &>(m).Lock();}
    void WriteLock(lock_type & m) const{m.Lock();}
    void ReadToWrite(lock_type &) const{}
    void WriteToRead(const lock_type &) const{}
};

template<>struct CLockAdapter<CRWLock>
{
    typedef CRWLock     lock_type;
    void Unlock(const lock_type & m) const{m.Unlock();}
    void ReadLock(const lock_type & m) const{m.ReadLock();}
    void WriteLock(lock_type & m) const{m.WriteLock();}
    void ReadToWrite(lock_type & m) const{
        m.Unlock();
        m.WriteLock();
    }
    void WriteToRead(const lock_type & m) const{
        m.Unlock();
        m.ReadLock();
    }
};

template<>struct CLockAdapter<CSpinLock>
{
    typedef CSpinLock   lock_type;
    void Unlock(const lock_type & m) const{m.Unlock();}
    void ReadLock(const lock_type & m) const{const_cast<lock_type &>(m).Lock();}
    void WriteLock(lock_type & m) const{m.Lock();}
    void ReadToWrite(lock_type &) const{}
    void WriteToRead(const lock_type &) const{}
};

//guard for all lock types
template<class LockT>
class CGuard
{
    CGuard(const CGuard &);
    CGuard & operator =(const CGuard &);
public:
    typedef LockT                   lock_type;
    typedef CLockAdapter<lock_type> adapter_type;
    explicit CGuard(lock_type & r, bool bWrite = true):lock_(&r){
        bWrite ? adapter_type().WriteLock(r) : adapter_type().ReadLock(r);
    }
    explicit CGuard(lock_type * p, bool bWrite = true):lock_(p){
        if(lock_)
            bWrite ? adapter_type().WriteLock(*p) : adapter_type().ReadLock(*p);
    }
    explicit CGuard(const lock_type & r):lock_(&r){
        adapter_type().ReadLock(r);
    }
    explicit CGuard(const lock_type * p):lock_(p){
        adapter_type().ReadLock(*p);
    }
    ~CGuard(){
        if(lock_)
            adapter_type().Unlock(*lock_);
    }
    lock_type * GetLock() const{return lock_;}
private:
    const lock_type * const lock_;
};

struct COnceGuard
{
    typedef void (*__Function)(void);
    explicit COnceGuard(__Function func = 0)
        : func_(func)
        , once_(PTHREAD_ONCE_INIT)
    {}
    void RunOnce(__Function func = 0){
        pthread_once(&once_, (func ? func : func_));
    }
private:
    __Function      func_;
    pthread_once_t  once_;
};

NS_SERVER_END

#endif

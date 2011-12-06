#ifndef DOZERG_MUTEX_H_20070924
#define DOZERG_MUTEX_H_20070924

/*
    ��POSIX�����ƽ��м򵥵ķ�װ
    ����ʹ��,���صײ�ʵ��,������ֲ
        CMutex          ������
        CAttrMutex      �����ԵĻ�����
        CCondition      ��������
        CRWLock         ��д��
        CSpinLock       ������
        CLockAdapter    ��������
        CGuard          ������
        COnceGuard      ȷ��ĳЩ����ֻ����һ��
    History:
        20080520    ����COnceGuard
        20080916    CMutex::Lock()��CMutex::Unlock()��Ϊvoid����ֵ
                    ��DEBUGģʽ��ʹCMutex�ܹ�������������׳��쳣
        20080920    ����CSpinLock
                    ͳһ���е�guard����ΪCGuard����ģ���������������
                    ����CLockAdapter��ͳһ������Ϊ
        20081010    CGuard���캯����bWrite��Ĭ��ֵ�޸�Ϊtrue
                    �޸ĸ�������Ա������const����
                    ��CGuard���캯���и�������const���ԣ��Զ�ѡ���������
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
    explicit CMutex(int){}  //���������³�ʼ��mutex_�Ļ���
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
    //��ָ����timeMs�������������lock,����false
    bool TimeLock(U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return !pthread_mutex_timedlock(&mutex_, &ts);
    }
};

class CAttrMutex : public CMutex
{
protected:
    bool    pshared_;   //�Ƿ���̼乲��
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
//���º����ɹ�����true;ʧ�ܷ���false
    bool Signal(){
        return !pthread_cond_signal(&cond_);
    }
    bool Broadcast(){
        return !pthread_cond_broadcast(&cond_);
    }
    bool Wait(CMutex & m){
        return !pthread_cond_wait(&cond_, &m.mutex_);
    }
    //�ȴ�ָ����timeMs����
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
    //��ָ����timeMs�������������rdlock,����false
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
    //��ָ����timeMs�������������wrlock,����false
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

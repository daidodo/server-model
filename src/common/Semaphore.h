#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    对POSIX信号量进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CUnnamedSemaphore   POSIX无名信号量
        CSemaphore          SystemV信号量
//*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <errno.h>
#include <stdexcept>        //std::runtime_error
#include <Tools.h>          //Tools::ErrorMsg, Tools::GetTimespec

NS_SERVER_BEGIN

class CUnnamedSemaphore
{
    CUnnamedSemaphore(const CUnnamedSemaphore &);
    CUnnamedSemaphore & operator =(const CUnnamedSemaphore &);
public:
    CUnnamedSemaphore(int init_val = 0,bool pshared = false) throw(std::runtime_error){
        if(sem_init(&sem_,pshared,init_val) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    ~CUnnamedSemaphore(){
        sem_destroy(&sem_);
    }
    void Post() throw(std::runtime_error){
        if(sem_post(&sem_) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    void Wait() throw(std::runtime_error){
        if(sem_wait(&sem_) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    bool TryWait(){
        return (0 == sem_trywait(&sem_));
    }
#if _POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600
    //在指定的timeMs毫秒内如果不能获取信号量,返回false
    bool TimeWait(U32 timeMs){
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == sem_timedwait(&sem_, &ts));
    }
#endif
    //获取信号量的当前值；返回-1表示获取失败
    int GetVal() const{
        int ret = -1;
        if(0 == sem_getvalue(&sem_,&ret) && ret < 0)
            ret = 0;
        return ret;
    }
private:
    mutable sem_t sem_;
};

class CSemaphore
{
    CSemaphore(const CSemaphore &);
    CSemaphore & operator =(const CSemaphore &);
public:
    CSemaphore(key_t key, int sem_count, int sem_flags){
    }
};

NS_SERVER_END

#endif


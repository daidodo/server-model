#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    对POSIX信号量进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CSemaphore      信号量
//*/
    
#include <semaphore.h>
#include <errno.h>
#include <stdexcept>        //std::runtime_error
#include <common/Tools.h>   //Tools::ErrorMsg

class CSemaphore
{
    CSemaphore(const CSemaphore &);
    CSemaphore & operator =(const CSemaphore &);
public:
    CSemaphore(int init_val = 0,bool pshared = false) throw(std::runtime_error){
        if(sem_init(&sem_,pshared,init_val) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    ~CSemaphore(){
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
        return !sem_trywait(&sem_);
    }
    //在指定的timeMs毫秒内如果不能获取信号量,返回false
    bool TimeWait(U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs,ts);
        return !sem_timedwait(&sem_,&ts);
    }
    //获取信号量的当前值；返回-1表示获取失败
    int GetVal() const{
        int ret = -1;
        if(!sem_getvalue(&sem_,&ret) && ret < 0)
            ret = 0;
        return ret;
    }
private:
    mutable sem_t sem_;
};

#endif


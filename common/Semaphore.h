#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    ��POSIX�ź������м򵥵ķ�װ
    ����ʹ��,���صײ�ʵ��,������ֲ
        CSemaphore      �ź���
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
    //��ָ����timeMs������������ܻ�ȡ�ź���,����false
    bool TimeWait(U32 timeMs){
        timespec ts;
        Tools::GetTimespec(timeMs,ts);
        return !sem_timedwait(&sem_,&ts);
    }
    //��ȡ�ź����ĵ�ǰֵ������-1��ʾ��ȡʧ��
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


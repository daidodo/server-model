#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    对POSIX信号量进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CSemaphore      POSIX信号量
//*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <stdexcept>        //std::runtime_error
#include <Tools.h>          //Tools::ErrorMsg, Tools::GetTimespec

NS_SERVER_BEGIN

class CSemaphore
{
    CSemaphore(const CSemaphore &);
    CSemaphore & operator =(const CSemaphore &);
public:
    //remove named semaphore
    static bool Unlink(const char * name){
        return (0 == sem_unlink(name));
    }
    //init unnamed semaphore
    //init_val: init value of Semaphore
    //pshared: shared in process(true) or not
    CSemaphore(int init_val = 0, bool pshared = false) throw(std::runtime_error)
        : semp_(0)
    {
        if(sem_init(&sem_, pshared, init_val) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
        semp_ = &sem_;
    }
    //init named semaphore
    //name: file name of Semaphore
    //init_val: init value of Semaphore
    //oflag: for sem_open()
    //mode: for sem_open()
    CSemaphore(const char * name
            , unsigned int init_val = 0
            , int oflag = O_CREAT
            , mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
        throw(std::runtime_error)
        : semp_(0)
    {
        sem_t * s = sem_open(name, oflag, mode, init_val);
        if(SEM_FAILED == s)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
        semp_ = s;
    }
    ~CSemaphore(){
        if(semp_ == &sem_){
            sem_destroy(&sem_);
        }else if(semp_)
            sem_close(semp_);
        semp_ = 0;
    }
    void Post() throw(std::runtime_error){
        if(sem_post(semp_) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    void Wait() throw(std::runtime_error){
        if(sem_wait(semp_) < 0)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    bool TryWait(){
        return (0 == sem_trywait(semp_));
    }
#ifdef __API_HAS_SEM_TIMEWAIT
    //在指定的timeMs毫秒内如果不能获取信号量,返回false
    bool TimeWait(U32 timeMs){
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == sem_timedwait(semp_, &ts));
    }
#endif
    //获取信号量的当前值；返回-1表示获取失败
    int GetVal() const{
        int ret = -1;
        if(0 == sem_getvalue(semp_, &ret) && ret < 0)
            ret = 0;
        return ret;
    }
private:
    sem_t * semp_;
    sem_t sem_;
};

NS_SERVER_END

#endif


#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    对POSIX信号量进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CSemaphore          POSIX信号量
        CXsiSemaphoreArray  XSI信号量集
//*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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
    static const int MODE_DEFAULT = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
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
            , mode_t mode = MODE_DEFAULT)
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

struct CSxiSemOperation
{
public:
    //add an operation
    //index: index in semaphore array
    //op: increment or decrement for semval (if ZERO, nothing will be added)
    //wait: wait or not if operaton suspended
    //undo: undo or not when process exit
    void AddOp(int index, int op, bool wait, bool undo){
        if(!op)
            return;     //ignore op == 0
        ops_.push_back(sembuf());
        sembuf & sb = ops_.back();
        sb.sem_num = index;
        sb.sem_op = op;
        sb.sem_flg = 0;
        if(!wait)
            sb.sem_flg |= IPC_NOWAIT;
        if(undo)
            sb.sem_flg |= SEM_UNDO;
    }
    //add waiting for semval == 0
    void AddWait(int index, bool wait){
        ops_.push_back(sembuf());
        sembuf & sb = ops_.back();
        sb.sem_num = index;
        sb.sem_op = 0;
        sb.sem_flg = (wait ? 0 : IPC_NOWAIT);
    }
private:
    std::vector<sembuf> ops_;
};

struct CXsiSemaphore
{
    CXsiSemaphore(const int & semid, size_t index)
        : semid_(semid)
        , index_(index)
    {}
    //apply one operation on semaphore
    bool Apply(int op, bool wait, bool undo);
    //apply operations on semaphore
    bool Apply(const CSxiSemOperation & ops);
    //set semval = value
    bool SetVal(int value);
    //get semval
    int GetVal() const;

private:
    const int & semid_;
    size_t index_;
};

class CXsiSemaphoreArray
{
    CXsiSemaphoreArray(const CXsiSemaphoreArray &);
    CXsiSemaphoreArray & operator =(const CXsiSemaphoreArray &);
    union semun{
        int val;
        struct semid_ds * buf;
        unsigned short  * array;
    }arg;
public:
    static const int MODE_DEFAULT = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    CXsiSemaphoreArray(key_t key
            , int nsems = 0
            , int init_val = 0
            , int oflags = O_CREAT
            , mode_t mode = MODE_DEFAULT)
        throw(std::runtime_error)
        : semid_(-1)
    {
        if(!init(key, nsems, oflags | mode, init_val))
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }

    CXsiSemaphoreArray(const char * pathname
            , int project
            , int nsems = 0
            , int init_val = 0
            , int oflags = O_CREAT
            , mode_t mode = MODE_DEFAULT)
        throw(std::runtime_error)
        : semid_(-1)
    {
    }

    bool IsValid() const{return semid_ >= 0;}
    //apply one operation on semaphore
    bool Apply(int index, int op, bool wait, bool undo);
    //apply operations on semaphore
    bool Apply(const CSxiSemOperation & ops);
    //get all semvals of semaphore array
    bool GetAll(std::vector<unsigned short> & results) const;
    //set all semvals of semaphore array
    bool SetAll(const std::vector<unsigned short> & values){
        if(!IsValid() || values.empty() || values.size() < nsems_)
            return false;
        struct{const unsigned short * array;} arg;
        arg.array = &values[0];
        if(0 > semctl(semid_, 0, SETALL, arg))
            return false;
        return true;
    }
    //destroy semaphore array
    void Destroy(){
        if(IsValid()){
            semctl(semid_, 0, IPC_RMID);
            semid_ = -1;
        }
    }
    //get semaphore array size
    size_t Size() const{return nsems_;}
    //access for semaphore array
    CXsiSemaphore operator [](size_t i){return CXsiSemaphore(semid_, i);}
private:
    bool init(key_t key, int nsems, int semflg, int init_val){
        assert(semid_ < 0);
        int id = semget(key, nsems, semflg);
        if(id < 0)
            return false;

        return true;
    }
    size_t getSize() const{
        assert(IsValid());
        struct semid_ds sem;
        struct{struct semid_ds * buf;} arg;
        arg.buf = &sem;
        if(0 > semctl(semid_, 0, IPC_STAT, arg))
            return 0;
        return sem.sem_nsems;
    }
    //members
    int semid_;
    size_t nsems_;
};

NS_SERVER_END

#endif


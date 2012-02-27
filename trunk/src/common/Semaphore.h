#ifndef DOZERG_SEMAPHORE_H_20090318
#define DOZERG_SEMAPHORE_H_20090318

/*
    对POSIX信号量进行简单的封装
    方便使用,隐藏底层实现,便于移植
        CSemaphore          POSIX信号量
        CXsiSemOperation    XSI信号量可以进行的操作
        CXsiSemaphore       XSI信号量
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

struct CXsiSemOperation
{
    static void GenOp(sembuf & sb, int index, int op, bool wait, bool undo){
        sb.sem_num = index;
        sb.sem_op = op;
        sb.sem_flg = 0;
        if(!wait)
            sb.sem_flg |= IPC_NOWAIT;
        if(undo)
            sb.sem_flg |= SEM_UNDO;
    }
    size_t Size() const{return ops_.size();}
    bool Empty() const{return ops_.empty();}
    const sembuf & operator [](size_t i) const{return ops_[i];}
    sembuf & operator [](size_t i){return ops_[i];}
    //add an operation
    //index: index in semaphore array
    //op: increment or decrement for semval (if ZERO, nothing will be added)
    //wait: wait or not if operaton suspended
    //undo: undo or not when process exit
    void AddOp(int index, int op, bool wait, bool undo){
        if(!op)
            return;     //ignore op == 0
        sembuf sb;
        GenOp(sb, index, op, wait, undo);
        ops_.push_back(sb);
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
    CXsiSemaphore(const int & semid, int index)
        : semid_(semid)
        , index_(index)
    {}
    bool IsValid() const{return semid_ >= 0 && index_ >= 0;}
    int Index() const{return index_;}
    //apply one operation on semaphore
    bool Apply(int op, bool wait, bool undo){
        if(!IsValid())
            return false;
        sembuf sb;
        CXsiSemOperation::GenOp(sb, index_, op, wait, undo);
        return (0 == semop(semid_, &sb, 1));
    }
    //apply operations on semaphore
    bool Apply(const CXsiSemOperation & ops){
        if(!IsValid())
            return false;
        if(ops.Empty())
            return true;
        std::vector<sembuf> sb;
        size_t sz = 0;
        sembuf * buf = filterOps(ops, sb, sz);
        if(!buf)
            return true;
        return (0 == semop(semid_, buf, sz));
    }
#ifdef __API_HAS_SEM_TIMEWAIT
    //apply one operation on semaphore in timeMs micro-seconds
    bool TimeApply(int op, bool wait, bool undo, U32 timeMs){
        if(!IsValid())
            return false;
        sembuf sb;
        CXsiSemOperation::GenOp(sb, index_, op, wait, undo);
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == semtimedop(semid_, &sb, 1, &ts));
    }
    //apply operations on semaphore in timeMs micro-seconds
    bool TimeApply(const CXsiSemOperation & ops, U32 timeMs){
        if(!IsValid())
            return false;
        if(ops.Empty())
            return true;
        std::vector<sembuf> sb;
        size_t sz = 0;
        sembuf * buf = filterOps(ops, sb, sz);
        if(!buf)
            return true;
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == semtimedop(semid_, buf, sz, &ts));
    }
#endif
    //set semval = value
    bool SetVal(int value){
        if(!IsValid())
            return false;
        struct{int val;} arg;
        arg.val = value;
        if(semctl(semid_, index_, SETVAL, arg) < 0)
            return false;
        return true;
    }
    //get semval
    //return: >=0:semval; <0:error
    int GetVal() const{
        if(!IsValid())
            return -1;
        return semctl(semid_, index_, GETVAL);
    }
private:
    //filter sembuf not for index_
    sembuf * filterOps(const CXsiSemOperation & ops, std::vector<sembuf> & sb, size_t & sz) const{
        assert(IsValid());
        bool skip = false;
        for(size_t i = 0;i < ops.Size();++i)
            if(ops[i].sem_num != index_){
                skip = true;
                break;
            }
        sembuf * buf = 0;
        sz = 0;
        if(skip){
            for(size_t i = 0;i < ops.Size();++i)
                if(ops[i].sem_num == index_)
                    sb.push_back(ops[i]);
            if(!sb.empty()){
                buf = &sb[0];
                sz = sb.size();
            }
        }else{
            buf = const_cast<sembuf *>(&ops[0]); //const_cast is for semop() prototype bug
            sz = ops.Size();
        }
        return buf;
    }
    //members
    const int & semid_;
    int index_;
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
    static const int FLAG_DEFAULT = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    CXsiSemaphoreArray(key_t key
            , int nsems = 0
            , int semflg = FLAG_DEFAULT)
        throw(std::runtime_error)
        : semid_(-1)
    {
        if(!init(key, nsems, semflg))
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    CXsiSemaphoreArray(const char * pathname
            , int project
            , int nsems = 0
            , int semflg = FLAG_DEFAULT)
        throw(std::runtime_error)
        : semid_(-1)
    {
        key_t key = ftok(pathname, project);
        if(-1 == key)
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
        if(!init(key, nsems, semflg))
            throw std::runtime_error(Tools::ErrorMsg(errno).c_str());
    }
    bool IsValid() const{return semid_ >= 0;}
    //get semaphore array size
    int Size() const{return nsems_;}
    //access for semaphore array
    CXsiSemaphore operator [](int i){return CXsiSemaphore(semid_, i);}
    //apply one operation on semaphore
    bool Apply(int index, int op, bool wait, bool undo){
        if(!IsValid())
            return false;
        sembuf sb;
        CXsiSemOperation::GenOp(sb, index, op, wait, undo);
        return (0 == semop(semid_, &sb, 1));
    }
    //apply operations on semaphore
    bool Apply(const CXsiSemOperation & ops){
        if(!IsValid())
            return false;
        if(ops.Empty())
            return true;
        return (0 == semop(semid_, const_cast<sembuf *>(&ops[0]), ops.Size())); //const_cast is for semop() prototype bug
    }
#ifdef __API_HAS_SEMTIMEDOP
    //apply one operation on semaphore in timeMs micro-seconds
    bool TimeApply(int index, int op, bool wait, bool undo, U32 timeMs){
        if(!IsValid())
            return false;
        sembuf sb;
        CXsiSemOperation::GenOp(sb, index, op, wait, undo);
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == semtimedop(semid_, &sb, 1, &ts));
    }
    //apply operations on semaphore in timeMs mirco-seconds
    bool TimeApply(const CXsiSemOperation & ops, U32 timeMs){
        if(!IsValid())
            return false;
        if(ops.Empty())
            return true;
        struct timespec ts;
        Tools::GetTimespec(timeMs, ts);
        return (0 == semtimedop(semid_, const_cast<sembuf *>(&ops[0]), ops.Size(), &ts)); //const_cast is for semop() prototype bug
    }
#endif
    //get all semvals of semaphore array
    bool GetAll(std::vector<unsigned short> & results) const{
        if(!IsValid() || !nsems_)
            return false;
        results.resize(nsems_);
        struct{unsigned short * array;} arg;
        arg.array = &results[0];
        if(0 > semctl(semid_, 0, GETALL, arg))
            return false;
        return true;
    }
    //set all semvals of semaphore array
    bool SetAll(const std::vector<unsigned short> & values){
        if(!IsValid() || values.empty() || int(values.size()) < nsems_)
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
private:
    bool init(key_t key, int nsems, int semflg){
        assert(semid_ < 0);
        int semid = semget(key, nsems, semflg);
        if(semid < 0)
            return false;
        if(!(nsems = getSize()))
            return false;
        semid_ = semid;
        nsems_ = nsems;
        return true;
    }
    int getSize() const{
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
    int nsems_;
};

NS_SERVER_END

#endif


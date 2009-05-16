#ifndef DOZERG_THREADS_H_20070905
#define DOZERG_THREADS_H_20070905

/*
    对POSIX线程的简单封装
        CActive         用于活跃线程数计数
        CThreads
        CThreadPool     线程池
        CThreadManager  自动伸缩的线程池
//*/

#include <errno.h>          //errno
#include <pthread.h>
#include <string>
#include <vector>
#include <common/Logger.h>
#include <common/LockInt.h> //CLockIntMax

NS_SERVER_BEGIN

template<class Int>
struct CActive{
    explicit CActive(Int & cnt):cnt_(cnt){++cnt_;}
    ~CActive(){--cnt_;}
private:
    Int & cnt_;
};

class CThreads
{
    typedef void * (*__ThreadProc)(void *);
    static void * threadProc(void * arg);
public:
    typedef CLockIntMax<size_t>     __ActiveCnt;
    typedef CActive<__ActiveCnt>    __Active;
    CThreads(__ThreadProc proc,int thread_count,size_t stack_sz = 16 << 10);
    //启动线程服务，repeat表示此服务是否自动重启
    //return +n(启动的线程数),-1(错误)
    int StartThreads(__DZ_STRING name,void * arg = 0,bool repeat = true);
    void WaitAll();
    size_t ThreadCount() const{return count_;}
    size_t ActiveCount() const{return activeCnt_.Value();}
    size_t ActiveMax() const{return activeCnt_.Max();}
    size_t ResetActiveMax(){return activeCnt_.ResetMax();}
    __ActiveCnt & Cnt(){return activeCnt_;}
private:
    __ThreadProc            proc_;
    void *                  proc_arg_;
    int                     count_;     //防止重复启动
    const size_t            stack_sz_;
    __DZ_VECTOR(pthread_t)  threads_;
    __ActiveCnt             activeCnt_;
    __DZ_STRING             name_;
    bool                    repeat_;
};

class CThreadPool
{
    static void * threadProc(void * arg);
public:
    typedef CThreads::__ActiveCnt   __ActiveCnt;
    typedef CThreads::__Active      __Active;
    explicit CThreadPool(size_t thread_count,size_t stack_sz = 16 << 10);
    virtual ~CThreadPool(){}
    virtual int StartThreads(__DZ_STRING name);
    virtual void WaitAll();
    size_t ThreadCount() const{return threads_.ThreadCount();}
    size_t ActiveCount() const{return threads_.ActiveCount();}
    size_t ActiveMax() const{return threads_.ActiveMax();}
    size_t ResetActiveMax(){return threads_.ResetActiveMax();}
    __ActiveCnt & Cnt(){return threads_.Cnt();}
protected:
    virtual int doIt() = 0;
private:
    CThreads    threads_;
};

template<class Que>
class CThreadManager
{
    typedef CLockInt<int,CSpinLock> __ThreadCount;
    typedef CActive<__ThreadCount>  __Active;
    typedef CSpinLock               __Lock;
    typedef CGuard<__Lock>          __Guard;
protected:
    typedef Que                             __Queue;
    typedef typename __Queue::value_type    __Job;
private:
    static const int SCHEDULE_INTERVAL_DEFAULT = 1; //s, 默认调度间隔
    static const int THREAD_COUNT_MIN_DEFAULT = 2;  //默认最少线程数
    static const int THREAD_COUNT_MAX_DEFAULT = 32; //默认最多线程数
    //worker thread
    static void * threadProc(void * arg){
        assert(arg);
        CThreadManager & self = *reinterpret_cast<CThreadManager *>(arg);
        for(__Job job;self.querySurvive();){
            self.inputQue_.Pop(job);
            __Active act(self.activeCount_);
            self.doIt(job);
        }
        --self.threadCount_;
        return 0;
    }
    //schedule thread
    static void * threadSchedule(void * arg){
        assert(arg);
        CThreadManager & self = *reinterpret_cast<CThreadManager *>(arg);
        for(;;){
            sleep(self.interval_);
            int active = self.activeCount_;
            int count = self.threadCount_;
            active = self.adjustThreadCount(active << 1);   //Expect
            if(active > count)  //need more threads
                self.addThreads(active - count);
            else{               //have waste threads
                count -= active;    //Waste
                if(count >= 5 && count >= (active >> 1))
                    self.deleteThread(count);
            }
        }
        return 0;
    }
public:
    CThreadManager(__Queue & input_que,size_t stack_sz = 16 << 10)
        : inputQue_(input_que)
        , stackSz_(stack_sz)
        , schedule_(0)
        , interval_(SCHEDULE_INTERVAL_DEFAULT)
        , deleteCount_(0)
        , threadCountMax_(THREAD_COUNT_MAX_DEFAULT)
        , threadCountMin_(THREAD_COUNT_MIN_DEFAULT)
    {}
    virtual ~CThreadManager(){}
    //name为服务线程的名字
    //thread_count为初始线程数，如果超过[threadCountMin_,threadCountMax_]的范围，则采用默认值
    //return +n(启动的线程数),-1(错误)
    virtual int StartThreads(__DZ_STRING name,int thread_count = 0){
        LOCAL_LOGGER(logger,"CThreadManager::StartThreads");
        if(Started())
            return -1;  //重复启动
        name_ = name;
        if(name_.empty())
            name_.push_back(' ');
        //start schedule thread
        if(pthread_create(&schedule_,0,threadSchedule,this)){
            FATAL_COUT(name_<<"'s schedule thread start failed"<<Tools::ErrorMsg(errno));
        }else{
            INFO(name_<<"'s schedule thread("<<schedule_<<") starts");
        }
        //start worker threads
        return addThreads(adjustThreadCount(thread_count));
    }
    virtual void WaitAll(){
        pthread_join(schedule_,0);
    }
    bool Started() const{return !name_.empty();}
    int ThreadCount() const{return threadCount_;}
    int ActiveCount() const{return activeCount_;}
    //设置调度线程的处理间隔时间（秒）
    void ScheduleInterval(int timeS){interval_ = timeS;}
    void ThreadCountMax(int thread_max){
        if(thread_max >= threadCountMin_)
            threadCountMax_ = thread_max;
    }
    void ThreadCountMin(int thread_min){
        if(thread_min <= threadCountMax_ && thread_min >= THREAD_COUNT_MIN_DEFAULT)
            threadCountMin_ = thread_min;
    }
    int ScheduleInterval() const{return interval_;}
    int ThreadCountMax() const{return threadCountMax_;}
    int ThreadCountMin() const{return threadCountMin_;}
protected:
    virtual void doIt(__Job &) = 0;
private:
    //return启动的线程数
    int addThreads(int thread_count){
        assert(thread_count > 0);
        LOCAL_LOGGER(logger,"CThreadManager::addThreads_aux");
        //set stack size
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr,stackSz_);
        pthread_t th = 0;
        int ret = 0;
        for(int i = 0;i < thread_count;++i){
            if(pthread_create(&th,&attr,threadProc,this)){
                ERROR_COUT("pthread_create error"<<Tools::ErrorMsg(errno));
            }else{
                pthread_detach(th);
                INFO(name_<<"("<<th<<") thread starts");
                ++ret;
            }
        }
        pthread_attr_destroy(&attr);
        threadCount_ += ret;
        return ret;
    }
    void deleteThread(int thread_count){
        assert(thread_count > 0);
        __Guard g(deleteLock_);
        deleteCount_ = thread_count;
    }
    bool querySurvive(){
        __Guard g(deleteLock_);
        assert(deleteCount_ >= 0);
        if(!deleteCount_)
            return true;
        --deleteCount_;
        return false;
    }
    int adjustThreadCount(int thread_count) const{
        const int t_min = threadCountMin_;
        const int t_max = threadCountMax_;
        if(thread_count < t_min)
            return t_min;
        if(thread_count > t_max)
            return t_max;
        return thread_count;
    }
    //fields:
    __Queue & inputQue_;
    const size_t stackSz_;
    __DZ_STRING name_;
    //schedule thread
    pthread_t   schedule_;
    int         interval_;  //s, 调度频率
    //用于删除多余线程
    CSpinLock   deleteLock_;
    int         deleteCount_;
    //thread counts
    __ThreadCount threadCount_;
    __ThreadCount activeCount_;
    volatile int threadCountMax_;
    volatile int threadCountMin_;
};

NS_SERVER_END

#endif

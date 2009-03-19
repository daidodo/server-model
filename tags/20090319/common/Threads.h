#ifndef DOZERG_THREADS_H_20070905
#define DOZERG_THREADS_H_20070905

/*
    对POSIX线程的简单封装
        CActive         用于活跃线程数计数
        CThreads
        CThreadPool     线程池
//*/

#include <pthread.h>
#include <string>
#include <vector>
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
    size_t                  stack_sz_;
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

NS_SERVER_END

#endif

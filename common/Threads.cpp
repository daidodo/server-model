#include <errno.h>          //errno
#include <exception>        //std::exception
#include <common/Logger.h>
#include "Threads.h"

NS_SERVER_BEGIN

#if defined(LOGGER) && !defined(LOGSYS)
GLOBAL_LOGGER(logger,"");
#endif

#ifndef NDEBUG
CMutex _G_cerr_mutex;   //global mutex for std::cerr
#endif

//class CThreads
void * CThreads::threadProc(void * arg)
{
    LOCAL_LOGGER(logger,"CThreads::threadProc");
    CThreads & self = *static_cast<CThreads *>(arg);
    for(;;sleep(10)){
        __DZ_TRY{
            self.proc_(self.proc_arg_);
        }__DZ_CATCH(std::exception,e){
            ERROR_COUT("exception - "<<e.what());
        }__DZ_CATCH_ALL{
            ERROR_COUT("unknown exception");
        }
        ERROR_COUT("thread "<<self.name_<<"("<<pthread_self()<<") returns");
        if(!self.repeat_)
            break;
    }
    return 0;
}

CThreads::CThreads(__ThreadProc proc,int thread_count,size_t stack_sz)
    : proc_(proc)
    , proc_arg_(0)
    , count_(thread_count)
    , stack_sz_(stack_sz)
    , repeat_(false)
{}

int CThreads::StartThreads(__DZ_STRING name,void * arg,bool repeat)
{
    if(threads_.empty()){
        LOCAL_LOGGER(logger,"CThreads::StartThreads");
        name_ = name;
        proc_arg_ = arg;
        repeat_ = repeat;
        //set stack size
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr,stack_sz_);
        threads_.resize(count_);
        int ret = 0;
        for(int i = 0;i < count_;++i){
            if(pthread_create(&threads_[i],&attr,threadProc,this)){
                ERROR_COUT("pthread_create error"<<Tools::ErrorMsg(errno));
                threads_[i] = 0;
            }else{
                ++ret;
                INFO(name<<"("<<threads_[i]<<") thread starts");
            }
        }
        pthread_attr_destroy(&attr);
        return ret;
    }
    return -1;  //ÖØ¸´Æô¶¯
}

void CThreads::WaitAll()
{
    for(int i = 0;i < count_;++i)
        if(threads_[i])
            pthread_join(threads_[i],0);
}

//class CThreadPool
void * CThreadPool::threadProc(void * arg)
{
    LOCAL_LOGGER(logger,"CThreadPool::threadProc");
    CThreadPool * self = reinterpret_cast<CThreadPool *>(arg);
    ASSERT(self,"arg is not valid <CThreadPool *>");
    int ret = self->doIt();
    ERROR_COUT("doIt() returns "<<ret);
    return 0;
}

CThreadPool::CThreadPool(size_t thread_count,size_t stack_sz)
    : threads_(threadProc,thread_count,stack_sz)
{}

int CThreadPool::StartThreads(__DZ_STRING name)
{
    return threads_.StartThreads(name,this,true);
}

void CThreadPool::WaitAll()
{
    threads_.WaitAll();
}

NS_SERVER_END

#include <common/Semaphore.h>

#include "comm.h"

static bool testSemOp(CSemaphore & sem)
{
    int v = sem.GetVal();
    if(1 != v){
        cerr<<"1: sem.GetVal()="<<v<<" is not 1\n";
        return false;
    }
    sem.Wait();
    v = sem.GetVal();
    if(0 != v){
        cerr<<"2: sem.GetVal()="<<v<<" is not 0\n";
        return false;
    }
    if(sem.TryWait()){
        cerr<<"sem.TryWait() should fail but succ\n";
        return false;
    }
    if(sem.TimeWait(1)){
        cerr<<"sem.TimeWait(1) should fail but succ\n";
        return false;
    }
    sem.Post();
    v = sem.GetVal();
    if(1 != v){
        cerr<<"3: sem.GetVal()="<<v<<" is not 1\n";
        return false;
    }
    return true;
}

static bool testSemaphore()
{
    CSemaphore sem_private(1);
    if(!testSemOp(sem_private)){
        cerr<<"testSemOp(sem_private) failed\n";
        return false;
    }
    CSemaphore sem_share(1, true);
    if(!testSemOp(sem_share)){
        cerr<<"testSemOp(sem_share) failed\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testSemaphore())
        return 1;
    cout<<"Semaphore test succ\n";
    return 0;
}

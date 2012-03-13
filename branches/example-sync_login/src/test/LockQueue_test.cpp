#include "comm.h"

#include <LockQueue.h>

static bool testLockQueue()
{
    typedef CLockQueue<int> __LockQue;
    __LockQue lockQue;
    for(int i = 1000;i < 2000;++i){
        if(!lockQue.Push(i)){
            cerr<<"lockQue.Push("<<i<<") failed\n";
            return false;
        }
    }
    if(1000 != lockQue.Size()){
        cerr<<"lockQue.Size()="<<lockQue.Size()<<" is not 1000\n";
        return false;
    }
    for(int i = 1000;i > 0;--i){
        int j = i - 1;
        if(!lockQue.PushFront(j)){
            cerr<<"lockQue.PushFront("<<j<<") failed\n";
            return false;
        }
    }
    if(2000 != lockQue.Size()){
        cerr<<"lockQue.Size()="<<lockQue.Size()<<" is not 2000\n";
        return false;
    }
    for(int i = 0;i < 1500;++i){
        int v;
        if(!lockQue.Pop(v)){
            cerr<<"1: lockQue.Pop() failed when LockQue.Size()="<<lockQue.Size()<<"\n";
            return false;
        }
        if(i != v){
            cerr<<"lockQue.Pop() get v="<<v<<" is not "<<i<<"\n";
            return false;
        }
    }
    __LockQue::container_type con;
    size_t oldSz = lockQue.Size();
    if(500 != oldSz){
        cerr<<"lockQue.Size()="<<oldSz<<" is not 500\n";
        return false;
    }
    if(!lockQue.PopAll(con)){
        cerr<<"lockQue.PopAll() failed\n";
        return false;
    }
    if(!lockQue.Empty()){
        cerr<<"lockQue.Empty() is false after PopAll\n";
        return false;
    }
    int v = 1500;
    for(__LockQue::container_type::const_iterator i = con.begin();i != con.end();++i, ++v){
        if(*i != v){
            cerr<<"*i="<<*i<<" is not "<<v<<" in con\n";
            return false;
        }
    }
    if(!lockQue.PushAll(con)){
        cerr<<"lockQue.PushAll(con) failed\n";
        return false;
    }
    if(oldSz != lockQue.Size()){
        cerr<<"lockQue.Size()="<<lockQue.Size()<<" is not oldSz="<<oldSz<<endl;
        return false;
    }
    for(int i = 1500;i < 2000;++i){
        int v;
        if(!lockQue.Pop(v)){
            cerr<<"2: lockQue.Pop() failed when LockQue.Size()="<<lockQue.Size()<<"\n";
            return false;
        }
        if(i != v){
            cerr<<"lockQue.Pop() get v="<<v<<" is not "<<i<<"\n";
            return false;
        }
    }
    if(!lockQue.Empty()){
        cerr<<"lockQue.Empty() is false after Pop all\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testLockQueue())
        return 1;
    cout<<"LockQueue test succ\n";
    return 0;
}

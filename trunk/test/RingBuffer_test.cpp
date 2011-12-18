#include <common/RingBuffer.h>

#include "comm.h"

static bool testRingBuf()
{
    CRingBuf<__DZ_STRING> ring(100);
    if(ring.Capacity() != 100){
        cerr<<"ring.Capacity()="<<ring.Capacity()<<" is not 100\n";
        return false;
    }
    __DZ_STRING s;
    for(int i = 0;i < 100;++i){
        s.push_back(i % 26 + 'a');
        if(!ring.Push(s)){
            cerr<<"ring.Push() element "<<i<<" failed\n";
            return false;
        }
    }
    if(ring.Size() != 100){
        cerr<<"ring.Size()="<<ring.Size()<<" is not 100\n";
        return false;
    }
    if(ring.Push(s)){
        cerr<<"ring.Push() succ when full\n";
        return false;
    }
    s.clear();
    for(int i = 0;i < 100;++i){
        __DZ_STRING v;
        if(!ring.Pop(v)){
            cerr<<"ring.Pop() element "<<i<<" failed\n";
            return false;
        }
        s.push_back(i % 26 + 'a');
        if(s != v){
            cerr<<"ring.Pop() element "<<i<<" is equal to '"<<v<<"' but '"<<s<<"'\n";
            return false;
        }
    }
    if(!ring.Empty()){
        cerr<<"ring.Empty() is false\n";
        return false;
    }
    if(ring.Pop(s)){
        cerr<<"ring.Pop() succ when empty\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testRingBuf())
        return 1;
    cout<<"RingBuffer test succ\n";
    return 0;
}

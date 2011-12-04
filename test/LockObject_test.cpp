#include <common/LockObject.h>

#include "comm.h"

static bool testLockObject()
{
    __DZ_STRING str;
    __DZ_STRING VAL;
    CLockObject1<__DZ_STRING, CMutex> lockStr(str);
    lockStr.Lock();
    for(int i = 0;i < 10;++i){
        lockStr->push_back('a' + i % 16);
        (*lockStr).push_back('a' + i % 16);
        VAL.push_back('a' + i % 16);
        VAL.push_back('a' + i % 16);
        if(VAL != *lockStr){
            cerr<<"1: lockStr='"<<*lockStr<<"' is not '"<<VAL<<"'\n";
            return false;
        }
    }
    for(int i = 0;i < 10;++i){
        lockStr.LockPointer()->push_back('a' + i % 26);
        VAL.push_back('a' + i % 26);
//        if(VAL != *lockStr.ReadPointer()){
//            cerr<<"2: lockStr='"<<*lockStr<<"' is not '"<<VAL<<"'\n";
//            return false;
//        }
    }
    return true;
}

int main()
{
    if(!testLockObject())
        return 1;
    cout<<"LockObject test succ\n";
    return 0;
}

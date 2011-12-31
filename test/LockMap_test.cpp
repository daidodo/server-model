#include "comm.h"

#include <common/LockMap.h>

static bool testLockSet()
{
    CLockSet<int> set;
    for(int i = 0;i < 1000;++i)
        set.insertL(i);
    if(set.Size() != 1000){
        cerr<<"set.Size()="<<set.Size()<<" is not 1000\n";
        return false;
    }
    for(int i = 0;i < 1000;++i){
        if(!set.findL(i)){
            cerr<<"1: set.findL("<<i<<") failed\n";
            return false;
        }
    }
    for(int i = 0;i < 500;++i){
        size_t c = set.eraseL(i);
        if(1 != c){
            cerr<<"set.eraseL("<<i<<")="<<c<<" is not 1\n";
            return false;
        }
    }
    if(set.Size() != 500){
        cerr<<"set.Size()="<<set.Size()<<" is not 500\n";
        return false;
    }
    for(int i = 0;i < 500;++i){
        if(set.findL(i)){
            cerr<<"set.findL("<<i<<") would fail but succ\n";
            return false;
        }
    }
    for(int i = 500;i < 1000;++i){
        if(!set.findL(i)){
            cerr<<"2: set.findL("<<i<<") failed\n";
            return false;
        }
    }
    return true;
}

static bool testLockMap()
{
    CLockMap<int, std::string> map;
    typedef CLockMap<int, std::string>::guard_type guard_type;
    std::string s;
    for(int i = 0;i < 100;++i){
        s.push_back('a' + i % 26);
        if(!map.insertL(make_pair(i, s))){
            cerr<<"map.insertL(value_type={"<<i<<", '"<<s<<"'}) failed\n";
            return false;
        }
    }
    if(map.Size() != 100){
        cerr<<"1: map.Size()="<<map.Size()<<" is not 100\n";
        return false;
    }
    for(int i = 100;i < 200;++i){
        s.push_back('a' + i % 26);
        if(!map.insertL(i, s)){
            cerr<<"map.insertL("<<i<<", '"<<s<<"') failed\n";
            return false;
        }
    }
    if(map.Size() != 200){
        cerr<<"map.Size()="<<map.Size()<<" is not 200\n";
        return false;
    }
    for(int i = 200;i > 100;--i){
        int j = i - 1;
        {
            guard_type g(map.GetLock());
            if(s != map[j]){
                cerr<<"map["<<j<<"]='"<<map[j]<<"' is not '"<<s<<"'\n";
                return false;
            }
        }
        map.eraseL(j);
        if(!s.empty())
            s.resize(s.size() - 1);
    }
    if(map.Size() != 100){
        cerr<<"2: map.Size()="<<map.Size()<<" is not 100\n";
        return false;
    }
    for(int i = 100;i > 0;--i){
        int j = i - 1;
        std::string v;
        if(!map.pickL(j, v)){
            cerr<<"map.pickL("<<j<<") return false\n";
            return false;
        }
        if(s != v){
            cerr<<"map["<<j<<"]='"<<v<<"' is not '"<<s<<"'\n";
            return false;
        }
        if(!s.empty())
            s.resize(s.size() - 1);
    }
    if(map.Size() != 0){
        cerr<<"map.Size()="<<map.Size()<<" is not 0\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testLockSet())
        return false;
    if(!testLockMap())
        return false;
    cout<<"LockMap test succ\n";
    return 0;
}

#include "comm.h"

#include <common/LockHashMap.h>

#define MAX_VAL     10000

static bool testSet()
{
    CLockHashSet<int> hashset;
    size_t sz;
    for(int i = sz = 0;i < MAX_VAL;++i)
        if(hashset.Insert(i))
            ++sz;
    if(hashset.Size() != sz){
        cerr<<"hashset.Size()="<<hashset.Size()<<" is not "<<sz<<endl;
        return false;
    }
    for(int i = MAX_VAL;i >= 0;--i){
        if(!hashset.Find(i))
            continue;
        size_t c = hashset.Erase(i);
        if(1 != c){
            cerr<<"hashset.Erase("<<i<<")="<<c<<" is not 1\n";
            return false;
        }
    }
    if(hashset.Size() != 0){
        cerr<<"hashset.Size()="<<hashset.Size()<<" is not 0\n";
        return false;
    }
    return true;
}

inline static std::string val(int i)
{
    std::string ret;
    for(int j = 0;j <= i;++i)
        ret.push_back('b' + j % 27);
    return ret;
}

static bool testMap()
{
    typedef CLockHashMap<int, std::string> __HashMap;
    __HashMap hashmap;
    std::string s;
    for(int i = 0;i < 100;++i){
        s.push_back('a' + i % 26);
        if(!hashmap.Insert(i, s)){
            cerr<<"hashmap.Insert("<<i<<", '"<<s<<"') returns false\n";
            return false;
        }
    }
    for(int i = 100;i < 200;++i){
        s.push_back('a' + i % 26);
        if(!hashmap.Insert(std::make_pair(i, s))){
            cerr<<"hashmap.Insert(value_type("<<i<<", '"<<s<<"')) returns false\n";
            return false;
        }
    }
    for(int i = 200;i < 300;++i){
        s.push_back('a' + i % 26);
        __HashMap::read_pointer xp;
        if(!hashmap.Insert(i, s, xp)){
            cerr<<"hashmap.Insert("<<i<<", '"<<s<<"', read_pointer) returns false\n";
            return false;
        }
        if(xp->first != i || xp->second != s){
            cerr<<"1: read_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<i<<", '"<<s<<"'}\n";
            return false;
        }
    }
    for(int i = 300;i < 400;++i){
        s.push_back('a' + i % 26);
        __HashMap::read_pointer xp;
        if(!hashmap.Insert(std::make_pair(i, s), xp)){
            cerr<<"hashmap.Insert(value_type("<<i<<", '"<<s<<"'), read_pointer) returns false\n";
            return false;
        }
        if(xp->first != i || xp->second != s){
            cerr<<"2: read_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<i<<", '"<<s<<"'}\n";
            return false;
        }
    }
    for(int i = 400;i < 500;++i){
        __HashMap::write_pointer xp;
        if(!hashmap.Insert(i, s, xp)){
            cerr<<"hashmap.Insert("<<i<<", '"<<s<<"', write_pointer) returns false\n";
            return false;
        }
        s.push_back('a' + i % 26);
        //xp->first = 0;    //there should be a compile error
        xp->second = s;
        if(xp->first != i || xp->second != s){
            cerr<<"1: write_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<i<<", '"<<s<<"'}\n";
            return false;
        }
    }
    for(int i = 500;i < 600;++i){
        __HashMap::write_pointer xp;
        if(!hashmap.Insert(std::make_pair(i, s), xp)){
            cerr<<"hashmap.Insert(value_type("<<i<<", '"<<s<<"'), write_pointer) returns false\n";
            return false;
        }
        s.push_back('a' + i % 26);
        //xp->first = 0;    //there should be a compile error
        xp->second = s;
        if(xp->first != i || xp->second != s){
            cerr<<"2: write_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<i<<", '"<<s<<"'}\n";
            return false;
        }
    }
    for(int i = 0;i < 600;++i)
        if(!hashmap.Find(i)){
            cerr<<"hashmap.Find("<<i<<") returns false\n";
            return false;
        }
    for(int i = 600;i < 1000;++i)
        if(hashmap.Find(i)){
            cerr<<"hashmap.Find("<<i<<") returns true\n";
            return false;
        }
    for(int i = 600;i > 0;--i){
        int j = i - 1;
        __HashMap::read_pointer xp;
        if(!hashmap.Find(j, xp)){
            cerr<<"hashmap.Find("<<j<<", read_pointer) returns false\n";
            return false;
        }
        if(xp->first != j || xp->second != s){
            cerr<<"3: read_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<j<<", '"<<s<<"'}\n";
            return false;
        }
        if(!s.empty())
            s.resize(s.size() - 1);
    }
    s = "a";
    for(int i = 600;i > 0;--i){
        int j = i - 1;
        __HashMap::write_pointer xp;
        if(!hashmap.Find(j, xp)){
            cerr<<"hashmap.Find("<<j<<", read_pointer) returns false\n";
            return false;
        }
        ++s[0];
        //xp->first = 0;    //there should be a compile error
        xp->second = s;
        if(xp->first != j || xp->second != s){
            cerr<<"3: write_pointer={"<<xp->first<<", '"<<xp->second<<"'} is not {"<<j<<", '"<<s<<"'}\n";
            return false;
        }
    }
    for(size_t i = 0;i < hashmap.BucketSize();++i){
        __HashMap::read_elem_array xparray;
        if(!hashmap.Iterate(i, xparray)){
            cerr<<"hashmap.Iterate("<<i<<", read_elem_array) returns false\n";
            return false;
        }
        if(xparray.empty()){
            cerr<<"read_elem_array is empty\n";
            return false;
        }
        for(size_t j = 0;j < xparray.size();++j){
            int k = xparray[j].first;
            std::string VAL = "a";
            VAL[0] += 600 - k;
            const std::string & v = xparray[j].second;
            if(1 != v.size()){
                cerr<<"v.size()="<<v.size()<<" is not 1\n";
                return false;
            }
            if(VAL != v){
                cerr<<"read_elem_array["<<i<<", "<<j<<"]={"<<k<<", "<<(UINT(v[0]) & 0xFF)<<"} is not "<<(UINT(VAL[0]) & 0xFF)<<"\n";
                return false;
            }
        }
    }
    for(size_t i = 0;i < hashmap.BucketSize();++i){
        __HashMap::write_elem_array xparray;
        if(!hashmap.Iterate(i, xparray)){
            cerr<<"hashmap.Iterate("<<i<<", write_elem_array) returns false\n";
            return false;
        }
        if(xparray.empty()){
            cerr<<"write_elem_array is empty\n";
            return false;
        }
        for(size_t j = 0;j < xparray.size();++j){
            int k = xparray[j].first;
            std::string VAL = "a";
            VAL[0] += k;
            //xparray[j].first = 0;   //there should be a compile error
            xparray[j].second = VAL;
        }
    }
    std::string VAL = "a";
    for(int i = 0;i < 600;++i){
        __HashMap::read_pointer xp;
        if(!hashmap.Find(i, xp)){
            cerr<<"hashmap.Find("<<i<<", read_pointer) failed\n";
            return false;
        }
        VAL[0] = 'a' + i;
        if(xp->second != VAL){
            cerr<<"read_pointer={"<<xp->first<<", "<<xp->second<<"} is not {"<<i<<", "<<VAL<<"}\n";
            return false;
        }
    }
    for(int i = 600;i < 700;++i){
        if(!hashmap.SetValue(i, "bcde")){
            cerr<<"hashmap.SetValue("<<i<<") failed\n";
            return false;
        }
    }
    VAL = "bcde";
    for(int i = 600;i < 700;++i){
        __HashMap::read_pointer xp;
        if(!hashmap.Find(i, xp)){
            cerr<<"hashmap.Find("<<i<<", read_pointer) failed\n";
            return false;
        }
        if(xp->second != VAL){
            cerr<<"read_pointer={"<<xp->first<<", "<<xp->second<<"} is not {"<<i<<", "<<VAL<<"}\n";
            return false;
        }
    }
    return true;
}

int main()
{
    srand(time(0));
    if(!testSet())
        return 1;
    if(!testMap())
        return 1;
    cout<<"LockHashMap test succ\n";
    return 0;
}

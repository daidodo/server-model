#include <common/FdMap.h>
#include "comm.h"

struct CTest
{
    static int objCount;
    __DZ_STRING a;
    static CTest * GetObject(){
        return new CTest;
    }
    static void PutObject(CTest *& p){
        delete p;
        p = 0;
    }
    template<class Ptr>
    static __DZ_STRING ToString(Ptr t){
        return (t ? "{'" + t->a + "'}" : "NULL");
    }
    CTest(){++objCount;}
    ~CTest(){--objCount;}
    CTest(const CTest & t):a(t.a){++objCount;}
    void Close(){}
    bool IsEqual(const __DZ_STRING & s) const{
        return a == s;
    }
};

int CTest::objCount = 0;

static bool testFdMap()
{
    CFdMap<__DZ_STRING> fdmap;
    __DZ_STRING s;
    for(int i = 0;i < 500;++i){
        s.push_back('a' + i % 26);
        fdmap[i] = s;
    }
    for(int i = 500;i > 0;--i){
        int j = i - 1;
        if(fdmap[j] != s){
            cerr<<"fdmap["<<j<<"]='"<<fdmap[j]<<"' is not '"<<s<<"'\n";
            return false;
        }
        if(!s.empty())
            s.resize(s.size() - 1);
    }
    return true;
}

template<class Ptr>
static bool testFdSockMap()
{
    CFdSockMap<CTest> fdmap;
    __DZ_STRING s100;
    for(int i = 0;i < 500;++i){
        s100.push_back('a' + i % 26);
        Ptr p = CTest::GetObject();
        p->a = s100;
        fdmap.SetSock(i, p);
    }
    __DZ_STRING s400;
    for(int i = 100;i < 500;++i){
        s400.push_back('b' + i % 27);
        Ptr p = CTest::GetObject();
        p->a = s400;
        fdmap.SetSock(i, p);
    }
    for(int i = 500;i > 450;--i){
        int j = i - 1;
        Ptr p = fdmap.GetSock(j);
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.GetSock("<<j<<")="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
    }
    for(int i = 450;i > 400;--i){
        int j = i - 1;
        Ptr p = 0;
        fdmap.GetSock(j, p);
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.GetSock("<<j<<", p) returns p="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
    }
    __DZ_VECTOR(int) fdvec;
    for(int i = 400;i < 350;--i)
        fdvec.push_back(i - 1);
    __DZ_VECTOR(Ptr) pvec(fdvec.size());
    fdmap.GetSock(fdvec.begin(), fdvec.end(), pvec.begin());
    int i = 400;
    for(typename __DZ_VECTOR(Ptr)::const_iterator it = pvec.begin();it != pvec.end();++it, --i){
        CTest * p = *it;
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.GetSock(from, to, dst) returns fdmap["<<i<<"]="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
    }


    if(CTest::objCount){
        cerr<<"CTest::objCount="<<CTest::objCount<<" is not 0\n";
    //    return false;
    }
    return true;
}

int main()
{
    if(!testFdMap())
        return 1;
    if(!testFdSockMap<CTest *>())
        return 1;
    //if(!testFdSockMap<CSharedPtr<CTest> >())
    //    return 1;
    cout<<"FdMap test succ\n";
    return 0;
}

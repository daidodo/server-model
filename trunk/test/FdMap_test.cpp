#include <common/FdMap.h>
#include "comm.h"

struct CTest
{
    typedef __DZ_ALLOC<CTest> allocator_type;
    static int objCount;
    __DZ_STRING a;
    static CTest * GetObject(){
        CTest * p = allocator_type().allocate(1);
        return new (p) CTest;
    }
    static void PutObject(CTest *& p){
        Tools::Destroy(p, allocator_type());
    }
    static void PutObject(CSharedPtr<CTest> & p){}
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

static inline bool checkSize(size_t size, size_t check, int pos, const char * head)
{
    assert(head);
    if(size != check){
        cerr<<pos<<": "<<head<<"="<<size<<" is not "<<check<<endl;
        return false;
    }
    if(size_t(CTest::objCount) != check){
        cerr<<pos<<": CTest::objCount="<<CTest::objCount<<" is not "<<check<<endl;
        return false;
    }
    return true;
}

template<class Ptr>
static bool testFdSockMap()
{
    CFdSockMap<CTest, Ptr> fdmap;
    __DZ_STRING s100;
    //SetSock 1
    for(int i = 0;i < 500;++i){
        s100.push_back('a' + i % 26);
        Ptr p = CTest::GetObject();
        p->a = s100;
        fdmap.SetSock(i, p);
    }
    if(!checkSize(fdmap.Size(), 500, 1, "fdmap.Size()"))
        return false;
    //SetSock 2
    __DZ_STRING s400;
    for(int i = 100;i < 500;++i){
        s400.push_back('b' + i % 27);
        Ptr p = CTest::GetObject();
        p->a = s400;
        fdmap.SetSock(i, p);
    }
    if(!checkSize(fdmap.Size(), 500, 2, "fdmap.Size()"))
        return false;
    //GetSock 1
    for(int i = 500;i > 450;--i){
        int j = i - 1;
        Ptr p = fdmap.GetSock(j);
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.GetSock("<<j<<")="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
        fdmap.SetSock(j, 0);
    }
    if(!checkSize(fdmap.Size(), 450, 3, "fdmap.Size()"))
        return false;
    //GetSock 2
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
        fdmap.SetSock(j, 0);
    }
    if(!checkSize(fdmap.Size(), 400, 4, "fdmap.Size()"))
        return false;
    //GetSock 3
    __DZ_VECTOR(int) fdvec;
    for(int i = 400;i > 350;--i)
        fdvec.push_back(i - 1);
    __DZ_VECTOR(Ptr) pvec(fdvec.size());
    fdmap.GetSock(fdvec.begin(), fdvec.end(), pvec.begin());
    int fd = 400 - 1;
    for(typename __DZ_VECTOR(Ptr)::const_iterator it = pvec.begin();it != pvec.end();++it, --fd){
        Ptr p = *it;
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.GetSock(from, to, dst) returns fdmap["<<fd<<"]="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
        fdmap.SetSock(fd, 0);
    }
    pvec.clear();
    if(!checkSize(fdmap.Size(), 350, 5, "fdmap.Size()"))
        return false;
    //CloseSock 1
    fdvec.clear();
    for(int i = 350;i > 300;--i){
        fdvec.push_back(i - 1);
        if(!s400.empty())
            s400.resize(s400.size() - 1);
    }
    fdmap.CloseSock(fdvec.begin(), fdvec.end());
    if(!checkSize(fdmap.Size(), 300, 6, "fdmap.Size()"))
        return false;
    //CloseSock 2
    fdvec.clear();
    for(int i = 300;i > 250;--i)
        fdvec.push_back(i - 1);
    pvec.resize(fdvec.size());
    fdmap.CloseSock(fdvec.begin(), fdvec.end(), pvec.begin());
    fd = 300 - 1;
    for(typename __DZ_VECTOR(Ptr)::const_iterator it = pvec.begin();it != pvec.end();++it, --fd){
        Ptr p = *it;
        if(!p || !p->IsEqual(s400)){
            cerr<<"fdmap.CloseSock(from, to, dst) returns fdmap["<<fd<<"]="<<CTest::ToString(p)<<" is not '"<<s400<<"'\n";
            return false;
        }
        if(!s400.empty())
            s400.resize(s400.size() - 1);
        CTest::PutObject(p);
    }
    pvec.clear();
    if(!checkSize(fdmap.Size(), 250, 7, "fdmap.Size()"))
        return false;
    //Clear
    fdmap.Clear();
    if(!checkSize(fdmap.Size(), 0, 8, "fdmap.Size()"))
        return false;
    return true;
}

int main()
{
    if(!testFdMap())
        return 1;
    if(!testFdSockMap<CTest *>())
        return 1;
    if(!testFdSockMap<CSharedPtr<CTest> >())
        return 1;
    cout<<"FdMap test succ\n";
    return 0;
}

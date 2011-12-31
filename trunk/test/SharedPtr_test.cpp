#include "comm.h"

#include <common/SharedPtr.h>

struct CTest
{
    typedef std::allocator<CTest> allocator_type;
    static int cnt;
    CTest(){++cnt;}
    explicit CTest(const std::string & s):s_(s){++cnt;}
    CTest(const CTest & a):s_(a.s_){++cnt;}
    ~CTest(){--cnt;}
    static CTest * GetObject(){
        CTest * ret = allocator_type().allocate(1);
        return new (ret) CTest;
    }
    static CTest * GetObject(const std::string & s){
        CTest * ret = allocator_type().allocate(1);
        return new (ret) CTest(s);
    }
    std::string s_;
};

int CTest::cnt = 0;

static bool testSharedPtr()
{
    typedef CSharedPtr<CTest> __Ptr;
    if(0 != CTest::cnt){
        cerr<<"1: CTest::cnt="<<CTest::cnt<<" is not 0\n";
        return false;
    }
    for(int i = 0;i < 100;++i){
        __Ptr p1 = CTest::GetObject();
        if(!p1){
            cerr<<"p1 == NULL\n";
            return false;
        }
        p1->s_ = "abcdef";
        if((*p1).s_ != "abcdef"){
            cerr<<"(*p1).s_='"<<(*p1).s_<<"' is not 'abcdef'\n";
            return false;
        }
        __Ptr p2 = p1;
        if(p2 != p1){
            cerr<<"p2 != p1\n";
            return false;
        }
        if(p2->s_ != "abcdef"){
            cerr<<"p2->s_='"<<p2->s_<<"' is not 'abcdef'\n";
            return false;
        }
        __Ptr p3 = CTest::GetObject("abcdef");
        if(p3 == p1){
            cerr<<"p3 == p1\n";
            return false;
        }
        if(p3->s_ != "abcdef"){
            cerr<<"p3->s_='"<<p3->s_<<"' is not 'abcdef'\n";
            return false;
        }
        if(p3 != &(*p3)){
            cerr<<"p3 != &(*p3)\n";
            return false;
        }
        if(p3 == &(*p1)){
            cerr<<"p3 == &(*p1)\n";
            return false;
        }
        p1.swap(p3);
        p1 = 0;
        if(p1){
            cerr<<"p1 != NULL\n";
            return false;
        }
    }
    if(0 != CTest::cnt){
        cerr<<"2: CTest::cnt="<<CTest::cnt<<" is not 0\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testSharedPtr())
        return 1;
    cout<<"SharedPtr test succ\n";
    return 0;
}

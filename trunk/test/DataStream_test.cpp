#include <common/DataStream.h>

#include "comm.h"

struct CTest
{
    long long a;
    unsigned long long b;
    long c;
    unsigned long d;
    int e;
    unsigned int f;
    short g;
    unsigned short h;
    char i;
    signed char j;
    unsigned char k;
    __DZ_STRING l;
    __DZ_VECTOR(int) m;
    char n[20];
    __DZ_VECTOR(__DZ_STRING) o;
    int p;
    bool operator ==(const CTest & t) const{
        return (a == t.a &&
                b == t.b &&
                c == t.c &&
                d == t.d &&
                e == t.e &&
                f == t.f &&
                g == t.g &&
                h == t.h &&
                i == t.i &&
                j == t.j &&
                k == t.k &&
                l == t.l &&
                m == t.m &&
                0 == memcmp(n, t.n, sizeof n) &&
                o == t.o &&
                p == t.p
               );
    }
    bool operator !=(const CTest & t) const{
        return !operator ==(t);
    }
};

static COutByteStream & operator <<(COutByteStream & obs, const CTest & t)
{
    size_t cur = obs.Size();
    obs<<t.a<<t.b<<t.c<<t.d<<t.e<<t.f<<t.g<<t.h<<t.i<<t.j<<t.k<<t.l
        <<Manip::array(&t.m[0], t.m.size())
        <<Manip::raw(t.n, sizeof t.n)
        <<t.o.size()
        <<Manip::range(t.o.begin(), t.o.end())
        <<Manip::insert(cur, t.p)
        ;
    return obs;
}

static CInByteStream & operator >>(CInByteStream & ibs, CTest & t)
{
    size_t cur = ibs.CurPos();
    ibs>>Manip::skip(sizeof t.p)
        >>t.a>>t.b>>t.c>>t.d>>t.e>>t.f>>t.g>>t.h>>t.i>>t.j>>t.k>>t.l;
    size_t sz = 100;
    t.m.resize(sz);
    ibs>>Manip::array(&t.m[0], t.m.size(), &sz);
    t.m.resize(sz);
    ibs>>Manip::raw(t.n, sizeof t.n);
    ibs>>sz;
    t.o.resize(sz);
    ibs>>Manip::range(t.o.begin(), t.o.end());
    ibs>>Manip::offset_value(cur, t.p);
    return ibs;
}

template<class Buf>
static bool testStreamInOut()
{
    const char str1[20] = "this is for test";
    __DZ_VECTOR(CTest) tests;
    COutByteStream obs;
    for(int i = 0;i < 10;++i){
        CTest t;
        t.a = 111;
        t.b = 222;
        t.c = 333;
        t.d = 444;
        t.e = 555;
        t.f = 666;
        t.g = 777;
        t.h = 888;
        t.i = 99;
        t.j = 100;
        t.k = 200;
        t.l = "300";
        for(int i = 0;i < 53;++i)
            t.m.push_back(i * i);
        memcpy(t.n, str1, sizeof t.n);
        __DZ_STRING str2;
        for(int i = 0;i < 30;++i){
            str2.push_back('a' + i);
            t.o.push_back(str2);
        }
        t.p = 400;

        if(!(obs<<t)){
            cerr<<"encode with COutByteStream failed\n";
            return false;
        }
        tests.push_back(t);
    }
    Buf buf;
    if(!obs.ExportData(buf)){
        cerr<<"COutByteStream::ExportData() failed\n";
        return false;
    }
    CInByteStream ibs(buf);
    for(int i = 0;i < 10;++i){
        CTest t2;
        if(!(ibs>>t2)){
            cerr<<"decode with CInByteStream failed\n";
            return false;
        }
        if(tests[i] != t2){
            cerr<<"encode and decode not match\n";
            return false;
        }
    }
    return true;
}

int main()
{
    if(!testStreamInOut<__DZ_STRING>())
        return -1;
    if(!testStreamInOut<__DZ_VECTOR(char)>())
        return -1;
    cout<<"DataStream test succ\n";
}

#include <common/DataStream.h>

#include "comm.h"

struct Test
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
    bool operator ==(const Test & t) const{
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
                l == t.l);
    }
    bool operator !=(const Test & t) const{
        return !operator ==(t);
    }
};

static inline COutByteStream & operator <<(COutByteStream & obs, const Test & t)
{
    return (obs<<t.a<<t.b<<t.c<<t.d<<t.e<<t.f<<t.g<<t.h<<t.i<<t.j<<t.k<<t.l);
}

static inline CInByteStream & operator >>(CInByteStream & ibs, Test & t)
{
    return (ibs>>t.a>>t.b>>t.c>>t.d>>t.e>>t.f>>t.g>>t.h>>t.i>>t.j>>t.k>>t.l);
}

int main()
{
    COutByteStream obs;
    Test t;
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
    if(!(obs<<t)){
        cerr<<"encode with COutByteStream failed\n";
        return -1;
    }
    __DZ_STRING buf;
    if(!obs.ExportData(buf)){
        cerr<<"COutByteStream::ExportData() failed\n";
        return -1;
    }
    CInByteStream ibs(buf);
    Test t2;
    if(!(ibs>>t2)){
        cerr<<"decode with CInByteStream failed\n";
        return -1;
    }
    if(t != t2){
        cerr<<"encode and decode not match\n";
        return -1;
    }



    cout<<"DataStream test succ\n";
    return 0;
}

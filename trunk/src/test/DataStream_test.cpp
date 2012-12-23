#include "comm.h"

#include <DataStream.h>

#define TEST_INSERT 1

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
    std::string l;
    std::vector<int> m;
    char n[20];
    std::vector<std::string> o;
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
    std::string toString() const{
        std::ostringstream oss;
        oss<<"{\na="<<a
            <<"\nb="<<b
            <<"\nc="<<c
            <<"\nd="<<d
            <<"\ne="<<e
            <<"\nf="<<f
            <<"\ng="<<g
            <<"\nh="<<h
            <<"\ni="<<int(i)
            <<"\nj="<<int(j)
            <<"\nk="<<int(k)
            <<"\nl="<<Tools::Dump(l)
            <<"\nm={";
        for(size_t index = 0;index < m.size();++index){
            if(index)
                oss<<", ";
            oss<<m[index];
        }
        oss<<"}\nn="<<Tools::DumpHex(n, sizeof n)
            <<"\no={";
        for(size_t index = 0;index < o.size();++index)
            oss<<"\n    "<<Tools::Dump(o[index]);
        oss<<"\n}\np="<<p
            <<"\n}";
        return oss.str();
    }
};

template<class OutStream>
static OutStream & operator <<(OutStream & obs, const CTest & t)
{
#if TEST_INSERT
    size_t cur = obs.Size();
#endif
    obs<<t.a<<t.b<<t.c<<t.d<<t.e<<t.f<<t.g<<t.h<<t.i<<t.j<<t.k<<t.l
        <<Manip::array(&t.m[0], t.m.size())
        <<Manip::raw(t.n, sizeof t.n)
        <<t.o.size()
        <<Manip::range(t.o.begin(), t.o.end())
#if TEST_INSERT
        <<Manip::insert(cur, t.p)
#endif
        ;
    return obs;
}

static CInByteStream & operator >>(CInByteStream & ibs, CTest & t)
{
#if TEST_INSERT
    size_t cur = ibs.CurPos();
    ibs>>Manip::skip(sizeof t.p);
#endif
    ibs>>t.a>>t.b>>t.c>>t.d>>t.e>>t.f>>t.g>>t.h>>t.i>>t.j>>t.k>>t.l;
    size_t sz = 100;
    t.m.resize(sz);
    ibs>>Manip::array(&t.m[0], t.m.size(), &sz);
    t.m.resize(sz);
    ibs>>Manip::raw(t.n, sizeof t.n);
    ibs>>sz;
    t.o.resize(sz);
    ibs>>Manip::range(t.o.begin(), t.o.end());
#if TEST_INSERT
    ibs>>Manip::offset_value(cur, t.p);
#endif
    return ibs;
}

template<class OutStream>
static bool testStreamInOut()
{
    const int COUNT = 10;
    const char str1[20] = "this is for test";
    std::vector<CTest> tests;
    OutStream obs;
    for(int i = 0;i < COUNT;++i){
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
        std::string str2;
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
    typename OutStream::__Buf buf;
    if(!obs.ExportData(buf)){
        cerr<<"COutByteStream::ExportData() failed\n";
        return false;
    }
    //cout<<"buf.size()="<<buf.size()<<endl;
    //cout<<"buf="<<Tools::Dump(buf)<<endl;
    CInByteStream ibs(buf);
    for(int i = 0;i < COUNT;++i){
        CTest t2;
        if(!(ibs>>t2)){
            cerr<<"decode with CInByteStream failed\n";
            return false;
        }
        if(tests[i] != t2){
            cerr<<"encode and decode not match, tests["<<i<<"]="<<tests[i].toString()<<", t2="<<t2.toString()<<endl;
            cerr<<"ibs.CurPos()="<<ibs.CurPos()<<", buf="<<Tools::DumpHex(&buf[0], ibs.CurPos())
                <<" | "
                <<Tools::DumpHex(&buf[ibs.CurPos()], buf.size() - ibs.CurPos(), ' ', false)
                <<"...\n";
            return false;
        }
    }
    return true;
}

template<class OutStream>
static bool testStreamInOutRef()
{
    const int COUNT = 10;
    const char str1[20] = "this is for test";
    std::vector<CTest> tests;
    typename OutStream::__Buf buf(100, 'a');
    OutStream obs(buf);
    for(int i = 0;i < COUNT;++i){
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
        std::string str2;
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
    if(!obs.ExportData()){
        cerr<<"COutByteStream::ExportData() failed\n";
        return false;
    }
    buf.erase(buf.begin(), buf.begin() + 100);
    //cout<<"buf="<<Tools::Dump(buf)<<endl;
    CInByteStream ibs(buf);
    for(int i = 0;i < COUNT;++i){
        CTest t2;
        if(!(ibs>>t2)){
            cerr<<"decode with CInByteStream failed\n";
            return false;
        }
        if(tests[i] != t2){
            cerr<<"encode and decode not match, tests["<<i<<"]="<<tests[i].toString()<<", t2="<<t2.toString()<<endl;
            cerr<<"ibs.CurPos()="<<ibs.CurPos()<<", buf="<<Tools::DumpHex(&buf[0], ibs.CurPos())
                <<" | "
                <<Tools::DumpHex(&buf[ibs.CurPos()], buf.size() - ibs.CurPos(), ' ', false)
                <<"...\n";
            return false;
        }
    }
    return true;
}

static bool testStreamInOutBuf()
{
    const int COUNT = 10;
    const char str1[20] = "this is for test";
    std::vector<CTest> tests;
    std::vector<char> buf(10 << 10);
    COutByteStreamBuf obs(&buf[0], buf.size());
    for(int i = 0;i < COUNT;++i){
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
        std::string str2;
        for(int i = 0;i < 30;++i){
            str2.push_back('a' + i);
            t.o.push_back(str2);
        }
        t.p = 400;

        if(!(obs<<t)){
            cerr<<"encode with COutByteStreamBuf failed\n";
            return false;
        }
        tests.push_back(t);
    }
    size_t sz = buf.size();
    if(!obs.ExportData(sz)){
        cerr<<"COutByteStreamBuf::ExportData() failed\n";
        return false;
    }
    CInByteStream ibs(&buf[0], sz);
    for(int i = 0;i < COUNT;++i){
        CTest t2;
        if(!(ibs>>t2)){
            cerr<<"decode with CInByteStream failed\n";
            return false;
        }
        if(tests[i] != t2){
            cerr<<"encode and decode not match, tests["<<i<<"]="<<tests[i].toString()<<", t2="<<t2.toString()<<endl;
            cerr<<"ibs.CurPos()="<<ibs.CurPos()<<", buf="<<Tools::DumpHex(&buf[0], ibs.CurPos())
                <<" | "
                <<Tools::DumpHex(&buf[ibs.CurPos()], buf.size() - ibs.CurPos(), ' ', false)
                <<"...\n";
            return false;
        }
    }
    return true;
}

int main()
{
    if(!testStreamInOut<COutByteStream>())
        return -1;
    if(!testStreamInOutRef<COutByteStreamStr>())
        return -1;
    if(!testStreamInOut<COutByteStreamVec>())
        return -1;
    if(!testStreamInOutRef<COutByteStreamVecRef>())
        return -1;
    if(!testStreamInOutBuf())
        return -1;
    cout<<"DataStream test succ\n";
}

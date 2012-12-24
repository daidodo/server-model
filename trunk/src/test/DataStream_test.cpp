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
    std::vector<int> m2;
    std::vector<int> m3;
    char n[20];
    std::vector<int> n2;
    std::vector<std::string> o;
    int p;
    int q;
    explicit CTest(int i = 0){
        a = 111 - i;
        b = 222 + i;
        c = 333 * i;
        d = 444 - i;
        e = 555 + i;
        f = 666 + i;
        g = 777 + i;
        h = 888 + i;
        i = 99 + i;
        j = 100 + i;
        k = 200 + i;
        l = "300";
        for(int ii = 0;ii < 53;++ii)
            m.push_back(ii * ii);
        m2 = m;
        m3 = m;
        const char str1[20] = "this is for test";
        memcpy(n, str1, sizeof n);
        n2 = m;
        std::string str2;
        for(int ii = 0;ii < 30;++ii){
            str2.push_back('a' + ii);
            o.push_back(str2);
        }
        p = 400 + i;
        q = 500 + i;
    }
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
                m2 == t.m2 &&
                m3 == t.m3 &&
                0 == memcmp(n, t.n, sizeof n) &&
                n2 == t.n2 &&
                o == t.o &&
                p == t.p &&
                q == t.q
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
        oss<<"\n}";
        oss<<"\nm2={";
        for(size_t index = 0;index < m2.size();++index){
            if(index)
                oss<<", ";
            oss<<m2[index];
        }
        oss<<"}\n";
        oss<<"\nm3={";
        for(size_t index = 0;index < m3.size();++index){
            if(index)
                oss<<", ";
            oss<<m3[index];
        }
        oss<<"}\n";
        oss<<"\nn="<<Tools::DumpHex(n, sizeof n);
        oss<<"\nn2={";
        for(size_t index = 0;index < n2.size();++index){
            if(index)
                oss<<", ";
            oss<<n2[index];
        }
        oss<<"}\n"
            <<"\no={";
        for(size_t index = 0;index < o.size();++index)
            oss<<"\n    "<<Tools::Dump(o[index]);
        oss<<"\n}";
        oss<<"\np="<<p
            <<"\nq="<<q
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
    obs<<t.a<<t.b<<t.c<<t.d<<t.e<<t.f<<t.g<<t.h<<t.i<<t.j<<t.k<<t.l;
    obs<<Manip::array(&t.m[0], uint64_t(t.m.size()));
    obs<<Manip::array<uint8_t>(t.m2);
    obs<<Manip::array(t.m3);
    obs<<Manip::raw(t.n, sizeof t.n);
    obs<<uint16_t(t.n2.size());
    obs<<Manip::raw(t.n2);
    obs<<t.o.size()
        <<Manip::raw(t.o.begin(), t.o.end())
#if TEST_INSERT
        <<Manip::insert(cur, t.p)
#endif
        <<Manip::value_byteorder(t.q, true)
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
    uint64_t sz = 100;
    t.m.resize(sz);
    ibs>>Manip::array(&t.m[0], uint64_t(t.m.size()), &sz);
    t.m.resize(sz);
    ibs>>Manip::array<uint8_t>(t.m2);
    ibs>>Manip::array(t.m3);
    ibs>>Manip::raw(t.n, sizeof t.n);
    uint16_t n2Len;
    ibs>>n2Len;
    ibs>>Manip::raw(t.n2, n2Len);
    size_t sz2;
    ibs>>sz2;
    t.o.resize(sz2);
    ibs>>Manip::raw(t.o.begin(), t.o.end());
#if TEST_INSERT
    ibs>>Manip::offset_value(cur, t.p);
#endif
    ibs>>Manip::value_byteorder(t.q, false);
    t.q = ntohl(t.q);
    return ibs;
}

template<class OutStream>
static bool testStreamInOut()
{
    const int COUNT = 10;
    std::vector<CTest> tests;
    OutStream obs;
    for(int i = 0;i < COUNT;++i){
        CTest t(i);

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
            cerr<<"1:decode with CInByteStream failed\n";
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
    std::vector<CTest> tests;
    typename OutStream::__Buf buf(100, 'a');
    OutStream obs(buf);
    for(int i = 0;i < COUNT;++i){
        CTest t(i);

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
            cerr<<"2:decode with CInByteStream failed\n";
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
    std::vector<CTest> tests;
    std::vector<char> buf(10 << 10);
    COutByteStreamBuf obs(&buf[0], buf.size());
    for(int i = 0;i < COUNT;++i){
        CTest t(i);

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
            cerr<<"3:decode with CInByteStream failed\n";
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
    if(!testStreamInOut<COutByteStreamStr>())
        return -1;
    if(!testStreamInOutRef<COutByteStreamStrRef>())
        return -1;
    if(!testStreamInOut<COutByteStreamVec>())
        return -1;
    if(!testStreamInOutRef<COutByteStreamVecRef>())
        return -1;
    if(!testStreamInOutBuf())
        return -1;
    cout<<"DataStream test succ\n";
}

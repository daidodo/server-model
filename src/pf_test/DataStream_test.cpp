#include <iostream>

#include <Tools.h>
#include <DataStream.h>

#include "sv_pack.h"

using namespace std;
using namespace NS_SERVER;

const size_t BUF_SZ = (100 << 20);

#define STRUCT(name)    \
    struct name \
    {   \
        char a; \
        signed char b;  \
        unsigned char c;    \
        short d;    \
        unsigned short e;   \
        int f;  \
        unsigned int g; \
        long h; \
        unsigned long i;    \
        long long j;    \
        unsigned long long k;   \
    }

#pragma pack(1)

STRUCT(A1);

#pragma pack()

STRUCT(A2);

inline void A1_to_A2(const A1 & a1, A2 & a2)
{
    a2.a = a1.a;
    a2.b = a1.b;
    a2.c = a1.c;
    a2.d = a1.d;
    a2.e = a1.e;
    a2.f = a1.f;
    a2.g = a1.g;
    a2.h = a1.h;
    a2.i = a1.i;
    a2.j = a1.j;
    a2.k = a1.k;
}

//encode/decode A1 using raw pointer
inline char * encode_A1(char * p, size_t len, const A1 & a)
{
    if(!p || len < sizeof(A1))
        return NULL;
    A1 * pa = reinterpret_cast<A1 *>(p);
    pa->a = a.a;
    pa->b = a.b;
    pa->c = a.c;
    pa->d = htons(a.d);
    pa->e = htons(a.e);
    pa->f = htonl(a.f);
    pa->g = htonl(a.g);
    pa->h = Tools::SwapByteOrder(a.h);
    pa->i = Tools::SwapByteOrder(a.i);
    pa->j = Tools::SwapByteOrder(a.j);
    pa->k = Tools::SwapByteOrder(a.k);
    return p + sizeof(A1);
}

inline const char * decode_A1(const char * p, size_t len, A1 & aa)
{
    if(!p || len < sizeof(A1))
        return NULL;
    const A1 & a = *reinterpret_cast<const A1 *>(p);
    A1 * pa = &aa;
    pa->a = a.a;
    pa->b = a.b;
    pa->c = a.c;
    pa->d = htons(a.d);
    pa->e = htons(a.e);
    pa->f = htonl(a.f);
    pa->g = htonl(a.g);
    pa->h = Tools::SwapByteOrder(a.h);
    pa->i = Tools::SwapByteOrder(a.i);
    pa->j = Tools::SwapByteOrder(a.j);
    pa->k = Tools::SwapByteOrder(a.k);
    return p + sizeof(A1);
}

//encode/decode A2 using data stream
inline COutByteStreamBuf & operator <<(COutByteStreamBuf & out, const A2 & a)
{
    return (out<<a.a
            <<a.b
            <<a.c
            <<a.d
            <<a.e
            <<a.f
            <<a.g
            <<a.h
            <<a.i
            <<a.j
            <<a.k);
}

inline CInByteStream & operator >>(CInByteStream & in, A2 & a)
{
    return (in>>a.a
            >>a.b
            >>a.c
            >>a.d
            >>a.e
            >>a.f
            >>a.g
            >>a.h
            >>a.i
            >>a.j
            >>a.k);
}

//encode/decode A2 using sv_pack APIs
inline int encode_A2(void ** ppCur, uint32_t * pdwLeft, const A2 & a)
{
    if(!ppCur || !pdwLeft)
        return -1;
    if(0 != C2_AddByte(ppCur, pdwLeft, a.a))
        return -1;
    if(0 != C2_AddByte(ppCur, pdwLeft, a.b))
        return -1;
    if(0 != C2_AddByte(ppCur, pdwLeft, a.c))
        return -1;
    if(0 != C2_AddWord(ppCur, pdwLeft, a.d))
        return -1;
    if(0 != C2_AddWord(ppCur, pdwLeft, a.e))
        return -1;
    if(0 != C2_AddDWord(ppCur, pdwLeft, a.f))
        return -1;
    if(0 != C2_AddDWord(ppCur, pdwLeft, a.g))
        return -1;
    if(sizeof(long) == 4){
        if(0 != C2_AddDWord(ppCur, pdwLeft, a.h))
            return -1;
        if(0 != C2_AddDWord(ppCur, pdwLeft, a.i))
            return -1;
    }else{
        if(0 != C2_AddQWord(ppCur, pdwLeft, a.h))
            return -1;
        if(0 != C2_AddQWord(ppCur, pdwLeft, a.i))
            return -1;
    }
    if(0 != C2_AddQWord(ppCur, pdwLeft, a.j))
        return -1;
    if(0 != C2_AddQWord(ppCur, pdwLeft, a.k))
        return -1;
    return 0;
}

inline int decode_A2(const void ** ppCur, uint32_t * pdwLeft, A2 & a)
{
    if(!ppCur || !pdwLeft)
        return -1;
    if(0 != C2_GetByte(ppCur, pdwLeft, (uint8_t *)&a.a))
        return -1;
    if(0 != C2_GetByte(ppCur, pdwLeft, (uint8_t *)&a.b))
        return -1;
    if(0 != C2_GetByte(ppCur, pdwLeft, &a.c))
        return -1;
    if(0 != C2_GetWord(ppCur, pdwLeft, (uint16_t *)&a.d))
        return -1;
    if(0 != C2_GetWord(ppCur, pdwLeft, &a.e))
        return -1;
    if(0 != C2_GetDWord(ppCur, pdwLeft, (uint32_t *)&a.f))
        return -1;
    if(0 != C2_GetDWord(ppCur, pdwLeft, &a.g))
        return -1;
    if(sizeof(long) == 4){
        if(0 != C2_GetDWord(ppCur, pdwLeft, (uint32_t *)&a.h))
            return -1;
        if(0 != C2_GetDWord(ppCur, pdwLeft, (uint32_t *)&a.i))
            return -1;
    }else{
        if(0 != C2_GetQWord(ppCur, pdwLeft, (uint64_t *)&a.h))
            return -1;
        if(0 != C2_GetQWord(ppCur, pdwLeft, (uint64_t *)&a.i))
            return -1;
    }
    if(0 != C2_GetQWord(ppCur, pdwLeft, (uint64_t *)&a.j))
        return -1;
    if(0 != C2_GetQWord(ppCur, pdwLeft, (uint64_t *)&a.k))
        return -1;
    return 0;
}

static bool testPod()
{
    const int COUNT = BUF_SZ / sizeof(A1);
#ifdef NDEBUG
    const int EX_CNT = 100;
#else
    const int EX_CNT = 1;
#endif
    A1 a1 = {
        12,     //a
        23,     //b
        34,     //c
        4567,   //d
        5678,   //e
        67890123,   //f
        78901234U,  //g
        89012345L,  //h
        90123456UL, //i
        123456789012LL, //j
        234567890123ULL,//k
    };
    A2 a2;
    A1_to_A2(a1, a2);
    cout<<"testPod()\n"
        <<"---Out---\n";

    std::string buf(BUF_SZ, 0);
    uint64_t start;

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        COutByteStreamBuf out(&buf[0], buf.size());
        for(int i = 0;i < COUNT;++i)
            out<<a2;
        if(!out){
            cerr<<"COutByteStream encode failed\n";
            return false;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"COutByteStream use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        char * p = &buf[0];
        size_t len = buf.size();
        for(int i = 0;i < COUNT;++i){
            p = encode_A1(p, len, a1);
            if(!p){
                cerr<<"raw pointer encode failed\n";
                return false;
            }
            len -= sizeof(A1);
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"raw pointer use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        void * pCur = &buf[0];
        uint32_t dwLeft = buf.size();
        for(int i = 0;i < COUNT;++i)
            if(0 != encode_A2(&pCur, &dwLeft, a2)){
                cerr<<"sv_pack encode failed\n";
                return false;
            }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"sv_pack use: "<<(start / 1000)<<" ms\n";

    cout<<"---In ---\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        CInByteStream in(&buf[0], buf.size());
        for(int i = 0;i < COUNT;++i)
            in>>a2;
        if(!in){
            cerr<<"CInByteStream encode failed\n";
            return false;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"CInByteStream use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        const char * p = &buf[0];
        size_t len = buf.size();
        for(int i = 0;i < COUNT;++i){
            p = decode_A1(p, len, a1);
            if(!p || (!a1.a && !a1.k)){
                cerr<<"raw pointer encode failed\n";
                return false;
            }
            len -= sizeof(A1);
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"raw pointer use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        const void * pCur = &buf[0];
        uint32_t dwLeft = buf.size();
        for(int i = 0;i < COUNT;++i)
            if(0 != decode_A2(&pCur, &dwLeft, a2)){
                cerr<<"sv_pack encode failed\n";
                return false;
            }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"sv_pack use: "<<(start / 1000)<<" ms\n";
    return true;
}

static bool testBuf()
{
    const size_t STR_SZ = 1000;
    const int COUNT = BUF_SZ / STR_SZ;
#ifdef NDEBUG
    const int EX_CNT = 500;
#else
    const int EX_CNT = 20;
#endif

    char str[STR_SZ];
    for(size_t i = 0;i < sizeof str;++i)
        str[i] = i;
    std::string buf(BUF_SZ, 0);
    uint64_t start;
    cout<<"\ntestBuf()\n"
        <<"---Out---\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        COutByteStreamBuf out(&buf[0], buf.size());
        for(int i = 0;i < COUNT;++i)
            out<<Manip::raw(str, sizeof str);
        if(!out){
            cerr<<"COutByteStreamBuf failed\n";
            return false;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"COutByteStreamBuf use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        char * p = &buf[0];
        size_t len = buf.size();
        for(int i = 0;i < COUNT;++i){
            if(len < sizeof str){
                cerr<<"raw pointer failed\n";
                return false;
            }
            memcpy(p, str, sizeof str);
            p += sizeof str;
            len -= sizeof str;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"raw pointer use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        void * pCur = &buf[0];
        uint32_t dwLen = buf.size();
        for(int i = 0;i < COUNT;++i){
            if(0 != C2_AddBuf(&pCur, &dwLen, str, sizeof str)){
                cerr<<"sv_pack failed\n";
                return false;
            }
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"sv_pack use: "<<(start / 1000)<<" ms\n";

    cout<<"---In ---\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        CInByteStream in(&buf[0], buf.size());
        for(int i = 0;i < COUNT;++i)
            in>>Manip::raw(str, sizeof str);
        if(!in){
            cerr<<"CInByteStreamBuf failed\n";
            return false;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"CInByteStreamBuf use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        const char * p = &buf[0];
        size_t len = buf.size();
        for(int i = 0;i < COUNT;++i){
            if(len < sizeof str){
                cerr<<"raw pointer failed\n";
                return false;
            }
            memcpy(str, p, sizeof str);
            p += sizeof str;
            len -= sizeof str;
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"raw pointer use: "<<(start / 1000)<<" ms\n";

    start = Tools::GetTimeUs();
    for(int c = 0;c < EX_CNT;++c){
        const void * pCur = &buf[0];
        uint32_t dwLen = buf.size();
        for(int i = 0;i < COUNT;++i){
            if(0 != C2_GetBuf(&pCur, &dwLen, str, sizeof str)){
                cerr<<"sv_pack failed\n";
                return false;
            }
        }
    }
    start = Tools::GetTimeUs() - start;
    cout<<"sv_pack use: "<<(start / 1000)<<" ms\n";

    return true;
}

int main()
{
    if(!testPod())
        return 1;
    if(!testBuf())
        return 1;
    return 0;
}

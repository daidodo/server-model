#include "comm.h"

#include <IterAdapter.h>

struct CTest
{
    std::string v1;
    int v2;
    CTest():v2(0){}
    CTest(const std::string & s, int i)
        : v1(s)
        , v2(i)
    {}
};

static std::string & ExtraV1(CTest & t){return t.v1;}
static const std::string & ExtraV1Const(const CTest & t){return t.v1;}

struct ExtraV2{
    typedef int result_type;
    result_type & operator ()(CTest & t) const{
        return t.v2;
    }
};

struct ExtraV2Const{
    typedef int result_type;
    const result_type & operator ()(const CTest & t) const{
        return t.v2;
    }
};

template<class Iter>
static void setV1(Iter first, Iter last)
{
    std::string s;
    for(int i = 0;first != last;++first, ++i){
        s.push_back('a' + i % 26);
        *first = s;
    }
}

template<class Iter>
static bool checkV1(Iter first, Iter last)
{
    std::string s;
    for(int i = 0;first != last;++first, ++i){
        s.push_back('a' + i % 26);
        if(s != *first){
            cerr<<"*first='"<<*first<<"' is not '"<<s<<"'\n";
            return false;
        }
    }
    return true;
}

template<class Iter>
static void setV2(Iter first, Iter last)
{
    for(int i = 0;first != last;++first, ++i)
        *first = i * i;
}

template<class Iter>
static bool checkV2(Iter first, Iter last)
{
    for(int i = 0;first != last;++first, ++i){
        if(*first != i * i){
            cerr<<"*first="<<*first<<" is not "<<i<<" * "<<i<<endl;
            return false;
        }
    }
    return true;
}

typedef std::vector<CTest> __Vec;

int main()
{
    __Vec vec(100);
    setV1(iter_adapt_fun<std::string>(vec.begin(), ExtraV1), iter_adapt_fun<std::string>(vec.end(), ExtraV1));
    if(!checkV1(const_iter_adapt_fun<std::string>(vec.begin(), ExtraV1Const),
                const_iter_adapt_fun<std::string>(vec.end(), ExtraV1Const)))
        return 1;
    __Vec:: const_iterator begin = vec.begin(), end = vec.end();
    if(!checkV1(const_iter_adapt_fun<std::string>(begin, ExtraV1Const),
                const_iter_adapt_fun<std::string>(end, ExtraV1Const)))
        return 1;
    setV2(iter_adapt(vec.begin(), ExtraV2()), iter_adapt(vec.end(), ExtraV2()));
    if(!checkV2(const_iter_adapt(vec.begin(), ExtraV2Const()),
                const_iter_adapt(vec.end(), ExtraV2Const())))
        return 1;
    begin = vec.begin();
    end = vec.end();
    if(!checkV2(const_iter_adapt(begin, ExtraV2Const()),
                const_iter_adapt(end, ExtraV2Const())))
        return 1;
    cout<<"IterAdapter test succ\n";
    return 0;
}

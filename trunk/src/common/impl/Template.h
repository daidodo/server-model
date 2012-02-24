#ifndef DOZERG_TEMPLATE_H_20091226
#define DOZERG_TEMPLATE_H_20091226

/*
    CTypeTraits
    CAssert
    COmitCV
    CIdentity
    CSelect1st
    CTypeSelector
    CByteOrderTraits
    HashFn
//*/

#include <byteswap.h>   //bswap_16, bswap_32, bswap_64

NS_IMPL_BEGIN

//struct CIntegerTraits
template<typename Integer>
struct CTypeTraits{
    static const bool CAN_MEMCPY = false;
};

#define __INTEGER_TRAITS_FOR_PODS(TYPE) template<>struct CTypeTraits<TYPE>{  \
    static const bool CAN_MEMCPY = true;    \
    static const int MAX_BITS = sizeof(TYPE) * 8;   \
}

__INTEGER_TRAITS_FOR_PODS(char);
__INTEGER_TRAITS_FOR_PODS(signed char);
__INTEGER_TRAITS_FOR_PODS(unsigned char);
__INTEGER_TRAITS_FOR_PODS(short);
__INTEGER_TRAITS_FOR_PODS(unsigned short);
__INTEGER_TRAITS_FOR_PODS(int);
__INTEGER_TRAITS_FOR_PODS(unsigned int);
__INTEGER_TRAITS_FOR_PODS(long);
__INTEGER_TRAITS_FOR_PODS(unsigned long);
__INTEGER_TRAITS_FOR_PODS(long long);
__INTEGER_TRAITS_FOR_PODS(unsigned long long);

#undef __INTEGER_TRAITS_FOR_PODS

//compilation assertion
template<bool>
struct CAssert{};

template<>
struct CAssert<true>{
    typedef int Result;
};

//omit const & volatile
template<class T>
struct COmitCV{
    typedef T result_type;
};

template<class T>
struct COmitCV<const T>{
    typedef T result_type;
};

template<class T>
struct COmitCV<volatile T>{
    typedef T result_type;
};

template<class T>
struct CIdentity : public std::unary_function<T,T> {
    T & operator()(T & v) const{return v;}
    const T & operator()(const T & v) const{return v;}
};

template<class Pair>
struct CSelect1st:public std::unary_function<Pair,typename Pair::first_type> {
    typename Pair::first_type & operator()(Pair & p) const{
        return p.first;
    }
    const typename Pair::first_type & operator()(const Pair & p) const {
        return p.first;
    }
};

//类型选择器
template<class T1,class T2,bool Sel>
struct CTypeSelector{
    typedef T1  RType;
};

template<class T1,class T2>
struct CTypeSelector<T1,T2,false>{
    typedef T2  RType;
};

//转字节序函数选择器
template<typename T, size_t N>
struct CByteOrderTraits{};

template<typename T>struct CByteOrderTraits<T, 1>{
    static T Swap(T a){
        return a;
    }
};

template<typename T>struct CByteOrderTraits<T, 2>{
    static T Swap(T a){
        return bswap_16(a);
    }
};

template<typename T>struct CByteOrderTraits<T, 4>{
    static T Swap(T a){
        return bswap_32(a);
    }
};

template<typename T>struct CByteOrderTraits<T, 8>{
    static T Swap(T a){
        return bswap_64(a);
    }
};

//hash函数集合
inline size_t __stl_hash_string(const char * s){
    size_t ret = 0;
    for(;s && *s;++s)
        ret = 5 * ret + *s;
    return ret;
}
inline size_t __stl_hash_string(const char * s,size_t sz){
    size_t ret = 0;
    for(size_t i = 0;i < sz;++s,++i)
        ret = 5 * ret + *s;
    return ret;
}
template<class Key>struct HashFn{
    size_t operator()(const Key & v) const{
        return v.HashFn();
    }
};
#define TEMPLATE_INSTANCE_FOR_TYPE(TYPE,HASH)   template<>struct HashFn<TYPE>{  \
    size_t operator()(TYPE v) const{return (HASH);}}
TEMPLATE_INSTANCE_FOR_TYPE(char *,__stl_hash_string(v));
TEMPLATE_INSTANCE_FOR_TYPE(const char *,__stl_hash_string(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed char *,__stl_hash_string((const char *)v));
TEMPLATE_INSTANCE_FOR_TYPE(const signed char *,__stl_hash_string((const char *)v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned char *,__stl_hash_string((const char *)v));
TEMPLATE_INSTANCE_FOR_TYPE(const unsigned char *,__stl_hash_string((const char *)v));
TEMPLATE_INSTANCE_FOR_TYPE(char,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed char,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned char,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed short,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned short,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed int,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned int,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed long,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned long,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(signed long long,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(unsigned long long,size_t(v));
TEMPLATE_INSTANCE_FOR_TYPE(std::string,__stl_hash_string(v.c_str(),v.length()));
#undef TEMPLATE_INSTANCE_FOR_TYPE

NS_IMPL_END

#endif

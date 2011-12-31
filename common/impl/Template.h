#ifndef DOZERG_TEMPLATE_H_20091226
#define DOZERG_TEMPLATE_H_20091226

#include <common/impl/Config.h>

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

NS_IMPL_END

#endif

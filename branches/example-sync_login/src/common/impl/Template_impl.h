#ifndef DOZERG_TEMPLATE_IMPL_H_20120228
#define DOZERG_TEMPLATE_IMPL_H_20120228

#include <impl/Config.h>

NS_IMPL_BEGIN

template<bool>
struct CAssert;

template<>
struct CAssert<true>{
    static const bool result = true;
};

#define __JOIN_TOKEN(a, b)  a##b
#define JOIN_TOKEN(a, b)    __JOIN_TOKEN(a, b)

#define __STATIC_ASSERT( token, expr )   enum { token = 1 / !!(expr) }

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

NS_IMPL_END

#endif


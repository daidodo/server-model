#ifndef DZ_TOOLS_IMPL_2001020
#define DZ_TOOLS_IMPL_2001020

#include <byteswap.h>   //bswap_16,bswap_32,bswap_64
#include <common/impl/Config.h>

NS_IMPL_BEGIN

template<typename T,size_t N>
struct CByteOrderTraits{};

template<typename T>struct CByteOrderTraits<T,1>{
    static T Swap(T a){
        return a;
    }
};

template<typename T>struct CByteOrderTraits<T,2>{
    static T Swap(T a){
        return bswap_16(a);
    }
};

template<typename T>struct CByteOrderTraits<T,4>{
    static T Swap(T a){
        return bswap_32(a);
    }
};

template<typename T>struct CByteOrderTraits<T,8>{
    static T Swap(T a){
        return bswap_64(a);
    }
};

NS_IMPL_END

#endif

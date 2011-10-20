#include <stdint.h>

//server namespaces
#define NS_SERVER       ServerModel
#define NS_IMPL         ServerModel_impl
//external lib namespace
#define NS_EXTERN_LIB   __DoZerg_External_Lib

#define NAMESAPCE_BEGIN(name)   namespace name{
#define NAMESAPCE_END           }

#define NS_SERVER_BEGIN     NAMESAPCE_BEGIN(NS_SERVER)
#define NS_SERVER_END       NAMESAPCE_END
#define NS_IMPL_BEGIN       NS_SERVER_BEGIN NAMESAPCE_BEGIN(NS_IMPL)
#define NS_IMPL_END         NAMESAPCE_END   NS_SERVER_END
#define NS_EXTLIB_BEGIN     NS_SERVER_BEGIN NAMESAPCE_BEGIN(NS_EXTERN_LIB)
#define NS_EXTLIB_END       NAMESAPCE_END   NS_SERVER_END

//basic types
typedef int8_t          S8;
typedef uint8_t         U8;
typedef int16_t         S16;
typedef uint16_t        U16;
typedef int32_t         S32;
typedef uint32_t        U32;
typedef int64_t         S64;
typedef uint64_t        U64;

#ifdef WIN32
#   define __DISPLAY_CODE   //used for display code in VisualStudio only
#endif

//use poll or epoll
#if defined(USEEPOLL)
#   define __USE_EPOLL  1
#else
#   define __USE_EPOLL  0
#endif

#ifdef NDEBUG
#   define __DZ_EXCEPTION
#endif

#ifdef __DZ_EXCEPTION
#   define __DZ_TRY         try
#   define __DZ_CATCH(t,e)  catch(t & e)
#   define __DZ_CATCH_ALL   catch(...)
#   define __DZ_RETHROW     throw
#   define __DZ_NOTHROW     throw()
#else
#   define __DZ_TRY
#   define __DZ_CATCH(t,e)  for(t e;0;)
#   define __DZ_CATCH_ALL   if(0)
#   define __DZ_RETHROW
#   define __DZ_NOTHROW
#endif


#ifndef _CXX_ALLOCATOR_H
#define _CXX_ALLOCATOR_H 1

//allocator choice
#ifdef __GNUC__
#   include <ext/mt_allocator.h>
#   define __glibcxx_base_allocator __gnu_cxx::__mt_alloc
#   define __USE_MT_ALLOC
#endif

#endif

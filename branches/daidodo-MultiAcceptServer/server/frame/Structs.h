#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

#include <vector>

#include "CmdSession.h"

NS_SERVER_BEGIN

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef __FdEventQue::container_type __FdEventList;

typedef std::vector<__SockPtr> __SockPtrList;
typedef std::vector<int> __FdArray;

NS_SERVER_END

#endif


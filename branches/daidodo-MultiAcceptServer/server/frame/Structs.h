#ifndef DOZERG_STRUCTS_H_20120110
#define DOZERG_STRUCTS_H_20120110

#include <vector>

#include "SockSession.h"
#include "CmdSession.h"

NS_SERVER_BEGIN

typedef CSockSession __SockSession;
typedef CSharedPtr<__SockSession> __SockPtr;

typedef CCmdSession __CmdSession;
typedef CFdSockMap<__SockSession, __SockPtr> __FdSockMap;

typedef CLockQueue<int> __FdQue;
typedef __FdQue::container_type __FdList;

typedef CLockQueue<__CmdSession *> __QueryCmdQue;
typedef __QueryCmdQue::container_type __QueryCmdList;

typedef CFdEvent __FdEvent;
typedef CLockQueue<__FdEvent> __FdEventQue;
typedef __FdEventQue::container_type __FdEventList;

typedef std::list<__SockPtr> __SockPtrList;
typedef std::vector<int> __FdArray;

NS_SERVER_END

#endif


#ifndef DZ_LOGSYS_LOG_SYSTEM_20071023
#define DZ_LOGSYS_LOG_SYSTEM_20071023

#if defined(LOGSYS)  || defined(__DISPLAY_CODE)
#   include "logsys_logger.h"
#   define LOGSYS_LOCK_WRITE(logger,level,msg)  \
        if(logger.Focus() || level >= logger.Level()){    \
        logger.Lock();logger.HeadInfo(level)<<msg<<"\n";  \
        logger.FlushStream();logger.Unlock();}
#   define INIT_LOGSYS(filename)           DZLOG::DZLOG_IMPL::CSingleWriter::Instance().Config(filename)
#   define GLOBAL_LOGSYS(logger,header)    DZLOG::CGlobalLogger logger(header)
#   define LOCAL_LOGSYS(logger,header)     DZLOG::CLocalLogger logger(header)
#   define SCOPE_LOGSYS(logger,header)     DZLOG::CScopeLogger logger(header)
#   define GLOBAL_LOGSYS_FOCUS(logger,header,expr)  DZLOG::CGlobalLogger logger(header,(expr))
#   define LOCAL_LOGSYS_FOCUS(logger,header,expr)   DZLOG::CLocalLogger logger(header,(expr))
#   define SCOPE_LOGSYS_FOCUS(logger,header,expr)   DZLOG::CScopeLogger logger(header,(expr))
#else
#   define LOGSYS_LOCK_WRITE(logger,level,msg)
#   define INIT_LOGSYS(filename)
#   define GLOBAL_LOGSYS(logger,header)
#   define LOCAL_LOGSYS(logger,header)
#   define SCOPE_LOGSYS(logger,header)
#   define GLOBAL_LOGSYS_FOCUS(logger,header,expr)
#   define LOCAL_LOGSYS_FOCUS(logger,header,expr)
#   define SCOPE_LOGSYS_FOCUS(logger,header,expr)
#endif

#define LOGSYS_TRACE(logger,msg)    LOGSYS_LOCK_WRITE(logger,0,msg)
#define LOGSYS_DEBUG(logger,msg)    LOGSYS_LOCK_WRITE(logger,1,msg)
#define LOGSYS_INFO(logger,msg)     LOGSYS_LOCK_WRITE(logger,2,msg)
#define LOGSYS_WARN(logger,msg)     LOGSYS_LOCK_WRITE(logger,3,msg)
#define LOGSYS_ERROR(logger,msg)    LOGSYS_LOCK_WRITE(logger,4,msg)
#define LOGSYS_FATAL(logger,msg)    LOGSYS_LOCK_WRITE(logger,5,msg)
//LOGSYS_OFF

#endif

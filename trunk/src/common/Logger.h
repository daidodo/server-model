#ifndef DOZERG_LOGGER_H_20071226
#define DOZERG_LOGGER_H_20071226

/*
    logsys和log4cpp的选择与使用定义
        INIT_LOGGER     初始化日志库
        DECL_LOGGER     在类的定义里申明静态日志成员
        IMPL_LOGGER     在类的实现里初始化申明过的日志成员
        GLOBAL_LOGGER   定义全局的日志对象，可多线程使用
        LOCAL_LOGGER    定义本地的日志对象，只能单线程使用(只logsys有效)
        SCOPE_LOGGER    定义局部的日志对象，只能单线程使用，适合短时间内写入大量日志，
                        在离开局部范围的时候才刷写到文件，需慎用(只logsys有效)
        LOGGER_TRACE    向日志对象写入各级别的日志
        ...
        TRACE           向变量名为logger的日志对象写入各级别的日志，方便使用
        ...
        ASSERT          使用日志的断言
        TRACE_COUT      将各级别信息写入日志文件和标准错误输出
        ...
    History:
        20081015        增加SCOPE_LOGGER
//*/

#if defined(LOGGER) || defined(__DISPLAY_CODE)
#   if defined(LOGSYS) || defined(__DISPLAY_CODE)
#       include <logsys/logsys_define.h>
#       define INIT_LOGGER(filename)	        INIT_LOGSYS(filename)
#       define DECL_LOGGER(logger)              static DZLOG::CGlobalLogger logger;
#       define IMPL_LOGGER(classname,logger)    DZLOG::CGlobalLogger classname::logger(#classname);
#       define GLOBAL_LOGGER(logger,header)     GLOBAL_LOGSYS(logger,header)
#       define LOCAL_LOGGER(logger,header)      LOCAL_LOGSYS(logger,header)
#       define SCOPE_LOGGER(logger,header)      SCOPE_LOGSYS(logger,header)
#       define LOGGER_TRACE(a,b)    LOGSYS_TRACE(a,b)
#       define LOGGER_DEBUG(a,b)    LOGSYS_DEBUG(a,b)
#       define LOGGER_INFO(a,b)     LOGSYS_INFO(a,b)
#       define LOGGER_WARN(a,b)     LOGSYS_WARN(a,b)
#       define LOGGER_ERROR(a,b)    LOGSYS_ERROR(a,b)
#       define LOGGER_FATAL(a,b)    LOGSYS_FATAL(a,b)
#   else
#       include <log4cplus/configurator.h>
#       include <log4cplus/logger.h>
#       define INIT_LOGGER(filename)            log4cplus::PropertyConfigurator::doConfigure(filename);
#       define DECL_LOGGER(logger)              static log4cplus::Logger logger;
#       define IMPL_LOGGER(classname, logger)   log4cplus::Logger classname::logger = log4cplus::Logger::getInstance(#classname);
#       define GLOBAL_LOGGER(logger,header)     log4cplus::Logger logger = log4cplus::Logger::getInstance(header);
#       define LOCAL_LOGGER(logger,header)
#       define SCOPE_LOGGER(logger,header)
#       define LOGGER_TRACE(a,b)    LOG4CPLUS_DEBUG(a,b)
#       define LOGGER_DEBUG(a,b)    LOG4CPLUS_DEBUG(a,b)
#       define LOGGER_INFO(a,b)     LOG4CPLUS_INFO(a,b)
#       define LOGGER_WARN(a,b)     LOG4CPLUS_WARN(a,b)
#       define LOGGER_ERROR(a,b)    LOG4CPLUS_ERROR(a,b)
#       define LOGGER_FATAL(a,b)    LOG4CPLUS_FATAL(a,b)
        NS_SERVER_BEGIN
        extern log4cplus::Logger logger;
        NS_SERVER_END
#   endif
#else
#   define INIT_LOGGER(filename)
#   define DECL_LOGGER(logger)
#   define IMPL_LOGGER(classname,logger)
#   define GLOBAL_LOGGER(logger,header)
#   define LOCAL_LOGGER(logger,header)
#   define SCOPE_LOGGER(logger,header)
#   define LOGGER_TRACE(a,b)
#   define LOGGER_DEBUG(a,b)
#   define LOGGER_INFO(a,b)
#   define LOGGER_WARN(a,b)
#   define LOGGER_ERROR(a,b)
#   define LOGGER_FATAL(a,b)
#endif

//defines for convenience use
#define TRACE(msg)  LOGGER_TRACE(logger,msg)
#define DEBUG(msg)  LOGGER_DEBUG(logger,msg)
#define INFO(msg)   LOGGER_INFO(logger,msg)
#define WARN(msg)   LOGGER_WARN(logger,msg)
#define ERROR(msg)  LOGGER_ERROR(logger,msg)
#define FATAL(msg)  LOGGER_FATAL(logger,msg)

#ifdef NDEBUG
#   define __COUT(msg)
#   define ASSERT(expr,msg)
#else
#   include <iostream>
#   include <Mutex.h>
NS_SERVER_BEGIN
extern CMutex _G_cerr_mutex;    //defined in <common/Threads.cpp>
NS_SERVER_END
#   define __COUT(msg)      {_G_cerr_mutex.Lock();std::cerr<<msg<<std::endl;_G_cerr_mutex.Unlock();}
#   define ASSERT(expr,msg) if(!(expr)){ERROR(msg<<" - "<<#expr);__COUT(msg<<" - "<<#expr);abort();}
#endif

#define TRACE_COUT(msg) {LOGGER_TRACE(logger,msg);__COUT(msg);}
#define DEBUG_COUT(msg) {LOGGER_DEBUG(logger,msg);__COUT(msg);}
#define INFO_COUT(msg)  {LOGGER_INFO(logger,msg);__COUT(msg);}
#define WARN_COUT(msg)  {LOGGER_WARN(logger,msg);__COUT(msg);}
#define ERROR_COUT(msg) {LOGGER_ERROR(logger,msg);__COUT(msg);}
#define FATAL_COUT(msg) {LOGGER_FATAL(logger,msg);__COUT(msg);}

#endif

#ifndef DZ_LOGSYS_CONFIG_20071019
#define DZ_LOGSYS_CONFIG_20071019

//namespaces
#define DZLOG       logsys
#define DZLOG_IMPL  logsys_impl

#define NAMESAPCE_BEGIN(name)   namespace name{
#define NAMESAPCE_END           }

#define DZLOG_BEGIN     NAMESAPCE_BEGIN(DZLOG)
#define DZLOG_END       NAMESAPCE_END
#define IMPL_BEGIN      DZLOG_BEGIN     NAMESAPCE_BEGIN(DZLOG_IMPL)
#define IMPL_END        NAMESAPCE_END   DZLOG_END

#endif

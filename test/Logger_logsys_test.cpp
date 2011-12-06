#ifndef LOGSYS
#   define LOGSYS
#endif
#include <common/Logger.h>

#include "comm.h"

class CTest
{
    DECL_LOGGER(logger);
public:
    void fun(){
        TRACE("this is trace info "<<1);
        DEBUG("this is debug info "<<2);
        WARN("this is warn info "<<3);
        ERROR("this is error info "<<4);
        FATAL("this is fatal info "<<5);
    }
};

IMPL_LOGGER(CTest, logger);

GLOBAL_LOGGER(g_logger, "Global");

static bool testLogger()
{
    INIT_LOGSYS("./logger.conf");
    CTest t;
    t.fun();
    //global logger
    LOGGER_TRACE(g_logger, "this is global trace msg "<<1);
    LOGGER_DEBUG(g_logger, "this is global debug msg "<<2);
    LOGGER_WARN(g_logger, "this is global warn msg "<<3);
    LOGGER_ERROR(g_logger, "this is global error msg "<<4);
    LOGGER_FATAL(g_logger, "this is global fatal msg "<<5);
    //local logger
    {
        LOCAL_LOGGER(logger, "Local");
        TRACE("this is local trace msg "<<1);
        DEBUG("this is local debug msg "<<2);
        WARN("this is local warn msg "<<3);
        ERROR("this is local error msg "<<4);
        FATAL("this is local fatal msg "<<5);
    }
    //scope logger
    for(int i = 0;i < 5;++i){
        SCOPE_LOGGER(logger, "Scope");
        TRACE("this is scope trace msg "<<1);
        DEBUG("this is scope debug msg "<<2);
        WARN("this is scope warn msg "<<3);
        ERROR("this is scope error msg "<<4);
        FATAL("this is scope fatal msg "<<5);
    }

    return true;
}

int main()
{
    if(!testLogger())
        return 1;
    cout<<"Logger test succ\n";
    return 0;
}

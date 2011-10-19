#ifndef DOZERG_LOGSYS_TOOLS_H_20071020
#define DOZERG_LOGSYS_TOOLS_H_20071020

//日志配置项和默认值

#include <time.h>
#include <sys/time.h>
#include <string>
#include <common/impl/Alloc.h>
#include "logsys_namespace.h"

IMPL_BEGIN

struct CConfigItems
{
    __DZ_STRING level_;
    __DZ_STRING filename_;
    size_t      filesz_;
    size_t      backIndex_;
    __DZ_STRING timeFormat_;
    size_t      buffersz_;
    bool        quickFlush_;
    CConfigItems()
        : level_("OFF")
        , filename_("output.log")
        , filesz_(500 * 1024)
        , backIndex_(5)
        , timeFormat_("%y-%m-%d %H:%M:%S")
        , buffersz_(4096)
        , quickFlush_(false)
    {}
};

IMPL_END

#endif

#include "comm.h"

#ifndef LOGSYS
#   define LOGSYS
#endif
#include <common/Logger.h>

#include "comm_logger.h"

int main()
{
    if(!testLogger())
        return 1;
    cout<<"Logger logsys test succ\n";
    return 0;
}

#include <common/SingleList.h>

#include "comm.h"

static bool testSingleList()
{
    return true;
}

int main()
{
    if(!testSingleList())
        return 1;
    cout<<"SingleList test succ\n";
    return 0;
}

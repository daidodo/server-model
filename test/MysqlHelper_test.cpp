#include <common/MysqlHelper.h>

#include "comm.h"

static bool testMySQL()
{
    CMySQL mysql;
    mysql.Connect("localhost", "test", "test@pass", "test");
    return true;
}

int main()
{
    cout<<"MysqlHelper test succ\n";
    return 0;
}

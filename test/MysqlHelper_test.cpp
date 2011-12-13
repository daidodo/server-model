#include <common/MysqlHelper.h>

#include "comm.h"

static bool testMySQL()
{
    CMySQL mysql;
    //connect
    mysql.Connect("localhost", "test", "test@pass", "test");
    if(!mysql.IsConnected() || !mysql){
        cerr<<"Connect() failed\n";
        return false;
    }
    if(!mysql.Ping()){
        cerr<<"Ping() failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    if(!mysql.SelectDB("test")){
        cerr<<"SelectDB('test') failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    //create table
    const char * sql = "create temporary table `test_table` (`name` varchar(16) not null, `value` int default 0, primary key(`name`))";
    if(!mysql.Query(sql)){
        cerr<<"Query(create table sql) failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    int i;
    if(0 != (i = mysql.AffectedRows())){
        cerr<<"AffectedRows()="<<i<<" is not 0, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    //insert

    return true;
}

int main()
{
    if(!testMySQL())
        return 1;
    cout<<"MysqlHelper test succ\n";
    return 0;
}

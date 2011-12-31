#include "comm.h"

#include <common/MysqlHelper.h>

static std::string name(int i)
{
    std::ostringstream oss;
    oss<<"item_"<<i;
    return oss.str();
}

static int value(int i)
{
    return i * i;
}

static bool checkRow(const CMyRow & row, int i)
{
    if(!row){
        cerr<<"RowNext() returns nothing for item_"<<i<<endl;
        return false;
    }
    int ret;
    if(2 != (ret = row.Size())){
        cerr<<"row.Size()="<<ret<<" is not 2 in result\n";
        return false;
    }
    std::string n(row[0], row.DataLength(0));
    if(n!= name(i)){
        cerr<<"row.name='"<<n<<"' is not '"<<name(i)<<"' for item_"<<i<<endl;
        return false;
    }
    int v = atoi(row[1]);
    if(v != value(i)){
        cerr<<"row.value="<<v<<" is not "<<value(i)<<" for item_"<<i<<endl;
        return false;
    }
    return true;
}

static bool checkCol(const CMyFieldInfo & col, const char * name)
{
    if(!col){
        cerr<<"col is nothing in result\n";
        return false;
    }
    if(name != col.Name()){
        cerr<<"col.Name()='"<<col.Name()<<"' is not '"<<name<<"' in result\n";
        return false;
    }
    return true;
}

static bool checkResult(CMyResult & result)
{
    if(!result){
        cerr<<"result is nothing\n";
        return false;
    }
    int ret;
    bool isStored = result.IsStored();
    if(isStored){
        if(100 != (ret = result.RowSize())){
            cerr<<"RowSize()="<<ret<<" is not 100 when isStored="<<isStored<<endl;
            return false;
        }
    }
    //row
    int end = (isStored ? 10 : 100);
    for(int i = 0;i < end;++i){
        if(!checkRow(result.RowNext(), i)){
            cerr<<"1: checkRow(i="<<i<<") failed when isStored="<<isStored<<endl;
            return false;
        }
    }
    for(int i = 100;i > end;--i){
        int j = i - 1;
        result.RowSeek(j);
        if(!checkRow(result.RowNext(), j)){
            cerr<<"2: checkRow(i="<<j<<") failed when isStored="<<isStored<<endl;
            return false;
        }
    }
    //col
    if(2 != (ret = result.FieldSize())){
        cerr<<"FieldSize()="<<ret<<" is not 2 when isStored="<<isStored<<endl;
        return false;
    }
    if(!checkCol(result.FieldInfo(0), "name")){
        cerr<<"checkCol('name') failed when isStored="<<isStored<<endl;
        return false;
    }
    if(!checkCol(result.FieldInfo(1), "value")){
        cerr<<"checkCol('value') failed when isStored="<<isStored<<endl;
        return false;
    }
    return true;
}

static bool testSelect(CMySQL & mysql)
{
    const char * sql = "select * from `test_table`";
    if(!mysql.Query(sql)){
        cerr<<"Query(select) failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    int ret;
    if(2 != (ret = mysql.LastFieldCount())){
        cerr<<"LastFieldCount()="<<ret<<" is not 2 after select, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    if(mysql.HasMultiResults()){
        cerr<<"HasMultiResults() should be false but true after select\n";
        return false;
    }
    if(0 != (ret = mysql.WarningCount())){
        cerr<<"WarningCount()="<<ret<<" is not 0 after select\n";
        return false;
    }
    return true;
}

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
    int ret;
    if(0 != (ret = mysql.AffectedRows())){
        cerr<<"AffectedRows()="<<ret<<" is not 0 after create table, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    //insert
    for(int i = 0;i < 100;++i){
        std::ostringstream oss;
        oss<<"insert into `test_table` value('"<<name(i)<<"', '"<<value(i)<<"')";
        sql = oss.str().c_str();
        if(!mysql.Query(sql)){
            cerr<<"Query(insert item_"<<i<<") failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
            return false;
        }
        if(1 != (ret = mysql.AffectedRows())){
            cerr<<"AffectedRows()="<<ret<<" is not 1 after insert item_"<<i<<", errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
            return false;
        }
    }
    //select
    if(!testSelect(mysql)){
        cerr<<"1: testSelect(1) failed\n";
        return false;
    }
    //store result
    CMyResult result;
    if(!mysql.StoreResult(result)){
        cerr<<"StoreResult() failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    if(!checkResult(result)){
        cerr<<"checkResult() failed after StoreResult()\n";
        return false;
    }
    result.Free();
    //select
    if(!testSelect(mysql)){
        cerr<<"2: testSelect(1) failed\n";
        return false;
    }
    //use result
    if(!mysql.UseResult(result)){
        cerr<<"UseResult() failed, errno="<<mysql.ErrorNo()<<": "<<mysql.ErrorMsg()<<endl;
        return false;
    }
    if(!checkResult(result)){
        cerr<<"checkResult() failed after UseResult()\n";
        return false;
    }
    return true;
}

int main()
{
    if(!testMySQL())
        return 1;
    cout<<"MysqlHelper test succ\n";
    return 0;
}

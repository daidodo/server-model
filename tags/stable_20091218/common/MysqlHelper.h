#ifndef DOZERG_MYSQL_HELPER_H_20080116
#define DOZERG_MYSQL_HELPER_H_20080116

/*
    对mysql进行简单包装
        CMySQL
        CMyResult
        CMyFieldInfo
        CMyCharacterSetInfo     mysql 5.1.10版本以后才有
//*/
#include <mysql.h>
#include <unistd.h>         //sleep
#include <string>
#include <cassert>
#include <common/Tools.h>   //Tools::DumpHex

NS_SERVER_BEGIN

#if MYSQL_VERSION_ID >= 40108 || (MYSQL_VERSION_ID >= 40023 && MYSQL_VERSION_ID < 40100)
#   define __HAS_MYSQL_HEX_STRING
#endif

#if MYSQL_VERSION_ID >= 50110
#   define __HAS_MYSQL_CHARSET_INFO
#endif

namespace MySQLHelp{
    //把(char *)0转化成""的辅助函数
    inline const char * _2str(const char * s){
        return s ? s : "";
    }
}//namespace MySQLHelp

class CMySQL;
class CMyResult;

//行
class CMyRow
{
    friend class CMyResult;
    MYSQL_ROW       row_;       //typedef char ** MYSQL_ROW;
    unsigned long * lengths_;   //各列数据的长度
    U32             size_;      //列数
public:
    CMyRow():row_(0),lengths_(0),size_(0){}
    //行是否有效(不为空)
    bool IsValid() const{return row_ != 0;}
    bool operator !() const{return !row_;}
    //列数
    U32 Size() const{
        assert(IsValid());
        return size_;
    }
    //指定列的数据长度
    U32 DataLength(U32 index) const{
        assert(IsValid());
        assert(index < Size());
        return lengths_[index];
    }
    //指定列的数据，可能是2进制字符
    const char * operator [](U32 index) const{
        assert(IsValid());
        assert(index < Size());
        return row_[index];
    }
};

//列信息
class CMyFieldInfo
{
    friend class CMyResult;
    MYSQL_FIELD * field_;
    CMyFieldInfo(MYSQL_FIELD * f):field_(f){}
public:
    CMyFieldInfo():field_(0){}
    //是否有效(不为空)
    bool IsValid() const{return field_ != 0;}
    bool operator !() const{return !field_;}
    //Name of column
    __DZ_STRING Name() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->name);
    }
    //Original column name, if an alias
    __DZ_STRING OrgName() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->org_name);
    }
    //Table of column if column was a field
    __DZ_STRING Table() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->table);
    }
    //Org table name, if table was an alias
    __DZ_STRING OrgTable() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->org_table);
    }
    //Org table name, if table was an alias
    __DZ_STRING DB() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->db);
    }
    //Catalog for table
    __DZ_STRING Catalog() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->catalog);
    }
    //Default value (set by mysql_list_fields)
    __DZ_STRING Default() const{
        assert(IsValid());
        return MySQLHelp::_2str(field_->def);
    }
    //Width of column (create length)
    U32 Length() const{
        assert(IsValid());
        return field_->length;
    }
    //Max width for selected set
    U32 MaxLength() const{
        assert(IsValid());
        return field_->max_length;
    }
    //Number of decimals in field
    U32 Decimals() const{
        assert(IsValid());
        return field_->decimals;
    }
    //Character set
    U32 CharacterSet() const{
        assert(IsValid());
        return field_->charsetnr;
    }
    //Type of field. See "mysql_com.h" for types
    enum enum_field_types Type() const{
        assert(IsValid());
        return field_->type;
    }
    //检测Div flags是否有flag指定的位标志
    //可用的flag标志参见"mysql_com.h"
    bool IsFlag(UINT flag) const{
        assert(IsValid());
        return (field_->flags & flag) != 0;
    }
};

//结果
class CMyResult
{
    friend class CMySQL;
    MYSQL_RES * res_;
    bool        stored_;    //是否是mysql_store_result返回的
public:
    CMyResult():res_(0),stored_(false){}
    ~CMyResult(){Free();}
    //结果是否有效(不为空)
    bool IsValid() const{return res_ != 0;}
    bool operator !() const{return !res_;}
    bool IsStored() const{return stored_;}
    //释放结果资源
    void Free(){
        if(IsValid()){
            mysql_free_result(res_);
            res_ = 0;
        }
    }
    //------------Row结果------------
    //在查询结果集中寻找任意行，offset为行号
    //只能在stored_ == true时使用
    void RowSeek(U32 offset){
        assert(IsValid());
        assert(IsStored());
        mysql_data_seek(res_,offset);
    }
    //返回结果中的行数
    //当stored_ == false时不返回正确的值，除非检索完结果中的所有行
    U32 RowSize() const{
        assert(IsValid());
        return mysql_num_rows(res_);
    }
    //返回结果的下一行
    CMyRow RowNext(){
        assert(IsValid());
        CMyRow ret;
        ret.row_ = mysql_fetch_row(res_);
        ret.lengths_ = mysql_fetch_lengths(res_);
        ret.size_ = FieldSize();
        return ret;
    }
    //------------Field信息------------
    //返回本行的列数
    U32 FieldSize() const{
        assert(IsValid());
        return mysql_num_fields(res_);
    }
    //返回指定列的字段定义信息
    CMyFieldInfo FieldInfo(U32 index){
        assert(IsValid());
        assert(index < FieldSize());
        return mysql_fetch_field_direct(res_,index);
    }

};

//字符集信息
#ifdef __HAS_MYSQL_CHARSET_INFO
class CMyCharacterSetInfo
{
    friend class CMySQL;
    MY_CHARSET_INFO cs_;
public:
    //character set number
    UINT CharacterSetNumber() const{return cs_.number;}
    //character set state
    UINT CharacterSetState() const{return cs_.state;}
    //character set name
    __DZ_STRING CharacterSetName() const{return MySQLHelp::_2str(cs_.name);}
    //collation name
    __DZ_STRING CollationName() const{return MySQLHelp::_2str(cs_.csname);}
    //character set directory
    __DZ_STRING Directory() const{return MySQLHelp::_2str(cs_.dir);}
    //min. length for multibyte strings
    int MultiByteCharacterMinLength() const{return cs_.mbminlen;}
    //max. length for multibyte strings
    int MultiByteCharacterMaxLength() const{return cs_.mbmaxlen;}
    //comment
    __DZ_STRING Comment() const{return MySQLHelp::_2str(cs_.comment);}
};
#endif

class CMySQL
{
    MYSQL * conn_;
public:
    //------------连接数据库------------
    CMySQL():conn_(0){}    //不进行连接
    explicit CMySQL(__DZ_STRING host,__DZ_STRING user = "",
        __DZ_STRING passwd = "",__DZ_STRING database = "")  //连接数据库
        : conn_(0)
    {
        Connect(host,user,passwd,database);
    }
    //关闭连接
    ~CMySQL(){Close();}
    //连接数据库
    void Connect(__DZ_STRING host = "",__DZ_STRING user = "",
        __DZ_STRING passwd = "",__DZ_STRING database = "")
    {
        Close();
        if((conn_ = mysql_init(0)) != 0){
            int i = 0;
            for(;i < 8 && !mysql_real_connect(conn_,host.c_str(),user.c_str(),passwd.c_str(),database.c_str(),0,0,0);++i)
                sleep(1);
            if(i >= 8)
                Close();
        }
    }
    //判断是否已连接
    bool IsConnected() const{return conn_ != 0;}
    bool operator !() const{return !conn_;}
    //检查与服务器的连接是否工作。如果连接丢失，将自动尝试再连接
    //返回连接最后是否有效
    bool Ping(){
        assert(IsConnected());
        return mysql_ping(conn_) == 0;
    }
    //关闭连接，释放conn_资源
    void Close(){
        if(IsConnected()){
            mysql_close(conn_);
            conn_ = 0;
        }
    }
    //------------查询------------
    //使由db指定的数据库成为默认数据库（当前数据库）
    //在后续查询中，该数据库将是未包含明确数据库区分符的表引用的默认数据库
    bool SelectDB(__DZ_STRING db){
        assert(IsConnected());
        return mysql_select_db(conn_,db.c_str()) == 0;
    }
    //执行sql语句
    bool Query(__DZ_STRING query){
        assert(IsConnected());
        return !mysql_real_query(conn_,query.c_str(),query.length());
    }
    //返回上次UPDATE更改的行数，上次DELETE删除的行数，或上次INSERT语句插入的行数
    //返回-1表示查询错误
    S32 AffectedRows() const{
        assert(IsConnected());
        my_ulonglong ret = mysql_affected_rows(conn_);
        if(ret == (my_ulonglong)-1)
            return -1;
        return S32(ret);
    }
    //返回最近查询结果的列数
    U32 LastFieldCount() const{
        assert(IsConnected());
        return mysql_field_count(conn_);
    }
    //返回由以前的INSERT或UPDATE语句为AUTO_INCREMENT列生成的值
    U32 LastInsertID() const{
        assert(IsConnected());
        return U32(mysql_insert_id(conn_));
    }
    //------------结果------------
    //将查询的全部结果读取到客户端
    //返回结果是否为空
    bool StoreResult(CMyResult & res){
        assert(IsConnected());
        res.Free();
        res.res_ = mysql_store_result(conn_);
        res.stored_ = true;
        return res.IsValid();
    }
    //将初始化结果集检索，但并不将结果集实际读取到客户端
    //返回结果是否为空
    bool UseResult(CMyResult & res){
        assert(IsConnected());
        res.Free();
        res.res_ = mysql_use_result(conn_);
        res.stored_ = false;
        return res.IsValid();
    }
    //返回当前执行的查询是否存在多个结果
    bool HasMultiResults() const{
        assert(IsConnected());
        return mysql_more_results(conn_);
    }
    //当存在多个结果时，使用StoreResult得到下一个结果
    //返回结果是否为空
    bool StoreNextResults(CMyResult & res){
        assert(IsConnected());
        if(mysql_next_result(conn_) == 0)
            return StoreResult(res);
        return false;
    }
    //当存在多个结果时，使用UseResult得到下一个结果
    //返回结果是否为空
    bool UseNextResults(CMyResult & res){
        assert(IsConnected());
        if(mysql_next_result(conn_) == 0)
            return UseResult(res);
        return false;
    }
    //------------错误与警告信息------------
    //返回执行前一个SQL语句生成的告警数目
    UINT WarningCount() const{
        assert(IsConnected());
        return mysql_warning_count(conn_);
    }
    //返回最近调用的API函数的错误消息
    //如果未出现错误，返回空字符串
    __DZ_STRING ErrorMsg() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_error(conn_));
    }
    //返回最近调用的API函数的错误代码，0表示未出现错误
    U32 ErrorNo() const{
        assert(IsConnected());
        return mysql_errno(conn_);
    }
    //------------字符串处理------------
    //创建可在SQL语句中使用的合法SQL字符串
    //需要转义的字符为NUL(ASCII 0)、‘\n’、‘\r’、‘\’、‘'’、‘"’、以及Control-Z
    __DZ_STRING EscapeString(__DZ_STRING str) const{
        assert(IsConnected());
        __DZ_STRING ret(str.length() * 2 + 1,0);
        U32 len = mysql_real_escape_string(conn_,&ret[0],str.c_str(),str.length());
        ret.resize(len);
        return ret;
    }
    //把字符串从形式上编码为十六进制格式，每个字符编码编码为2个十六进制数
    //以便可采用0xvalue或X'value'格式将字符串置于SQL语句中
    static __DZ_STRING HexString(__DZ_STRING str){
#ifdef __HAS_MYSQL_HEX_STRING
        __DZ_STRING ret(str.length() * 2 + 1,0);
        U32 len = mysql_hex_string(&ret[0],str.c_str(),str.length());
        ret.resize(len);
        return ret;
#else
        return Tools::DumpHex(str,0);
#endif
    }
    //------------字符集信息------------
    //获得客户端字符集的信息
#ifdef __HAS_MYSQL_CHARSET_INFO
    void GetCharacterSetInfo(CMyCharacterSetInfo & info) const{
        assert(IsConnected());
        mysql_get_character_set_info(conn_,&info.cs_);
    }
#endif
    //返回当前连接默认的字符集名字
    __DZ_STRING GetCharacterSetName() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_character_set_name(conn_));
    }
    //设置默认的字符集，字符串csname指定了1个有效的字符集名称
#ifdef __HAS_MYSQL_CHARSET_INFO
    bool SetCharacterSet(__DZ_STRING csname){
        assert(IsConnected());
        return mysql_set_character_set(conn_,csname.c_str()) == 0;
    }
#endif
    //------------版本，连接统计信息------------
    //返回表示客户端库版本的字符串，例如：5.0.45
    static __DZ_STRING GetClientInfo(){
        return MySQLHelp::_2str(mysql_get_client_info());
    }
    //返回表示客户端库版本的整数
    //值的格式是XYYZZ，其中X是主版本号，YY是发布级别，ZZ是发布级别内的版本号
    //例如，值40102表示客户端库的版本是4.1.2
    static U32 GetClientVersion(){
        return mysql_get_client_version();
    }
    //返回描述连接类型的字符串，包括服务器主机名
    //例如"Localhost via UNIX socket"
    __DZ_STRING GetHostInfo() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_get_host_info(conn_));
    }
    //返回当前连接所使用的协议版本
    //格式为：major_version*10000 + minor_version *100 + sub_version
    //例如：50045
    U32 GetProtoInfo() const{
        assert(IsConnected());
        return mysql_get_proto_info(conn_);
    }
    //以整数形式返回服务器的版本号
    //例如，对于5.0.12，返回500012
    U32 GetServerVersion() const{
        assert(IsConnected());
        return mysql_get_server_version(conn_);
    }
    //返回包含特定信息的字符串
    //包括以秒为单位的正常运行时间，以及运行线程的数目，问题数，再加载次数，以及打开的表数目
    //例如：
    //Uptime: 157911  Threads: 1  Questions: 5  Slow queries: 0  Opens: 12  Flush tables: 1  Open tables: 6  Queries per second avg: 0.000
    __DZ_STRING Stat() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_stat(conn_));
    }
    //------------mysql线程------------
    //返回当前连接的线程ID
    //如果连接丢失，并使用Ping()进行了再连接，线程ID将改变
    U32 ThreadID() const{
        assert(IsConnected());
        return mysql_thread_id(conn_);
    }
    //请求服务器杀死由pid指定的线程，返回是否成功
    bool Kill(U32 pid){
        assert(IsConnected());
        return mysql_kill(conn_,pid) == 0;
    }
    //------------事务处理------------
    //提交当前事务
    bool Commit(){
        assert(IsConnected());
        return mysql_commit(conn_) == 0;
    }
    //回滚当前事务
    bool Rollback(){
        assert(IsConnected());
        return mysql_rollback(conn_) == 0;
    }
    //启用/禁用autocommit模式
    //返回设置是否成功
    bool SetAutoCommit(bool enable = true){
        assert(IsConnected());
        return mysql_autocommit(conn_,enable ? 1 : 0) == 0;
    }
};

NS_SERVER_END

#endif

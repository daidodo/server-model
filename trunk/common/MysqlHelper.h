#ifndef DOZERG_MYSQL_HELPER_H_20080116
#define DOZERG_MYSQL_HELPER_H_20080116

/*
    ��mysql���м򵥰�װ
        CMySQL
        CMyResult
        CMyFieldInfo
        CMyCharacterSetInfo     mysql 5.1.10�汾�Ժ����
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
    //��(char *)0ת����""�ĸ�������
    inline const char * _2str(const char * s){
        return s ? s : "";
    }
}//namespace MySQLHelp

class CMySQL;
class CMyResult;

//��
class CMyRow
{
    friend class CMyResult;
    MYSQL_ROW       row_;       //typedef char ** MYSQL_ROW;
    unsigned long * lengths_;   //�������ݵĳ���
    U32             size_;      //����
public:
    CMyRow():row_(0),lengths_(0),size_(0){}
    //���Ƿ���Ч(��Ϊ��)
    bool IsValid() const{return row_ != 0;}
    bool operator !() const{return !row_;}
    //����
    U32 Size() const{
        assert(IsValid());
        return size_;
    }
    //ָ���е����ݳ���
    U32 DataLength(U32 index) const{
        assert(IsValid());
        assert(index < Size());
        return lengths_[index];
    }
    //ָ���е����ݣ�������2�����ַ�
    const char * operator [](U32 index) const{
        assert(IsValid());
        assert(index < Size());
        return row_[index];
    }
};

//����Ϣ
class CMyFieldInfo
{
    friend class CMyResult;
    MYSQL_FIELD * field_;
    CMyFieldInfo(MYSQL_FIELD * f):field_(f){}
public:
    CMyFieldInfo():field_(0){}
    //�Ƿ���Ч(��Ϊ��)
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
    //���Div flags�Ƿ���flagָ����λ��־
    //���õ�flag��־�μ�"mysql_com.h"
    bool IsFlag(UINT flag) const{
        assert(IsValid());
        return (field_->flags & flag) != 0;
    }
};

//���
class CMyResult
{
    friend class CMySQL;
    MYSQL_RES * res_;
    bool        stored_;    //�Ƿ���mysql_store_result���ص�
public:
    CMyResult():res_(0),stored_(false){}
    ~CMyResult(){Free();}
    //����Ƿ���Ч(��Ϊ��)
    bool IsValid() const{return res_ != 0;}
    bool operator !() const{return !res_;}
    bool IsStored() const{return stored_;}
    //�ͷŽ����Դ
    void Free(){
        if(IsValid()){
            mysql_free_result(res_);
            res_ = 0;
        }
    }
    //------------Row���------------
    //�ڲ�ѯ�������Ѱ�������У�offsetΪ�к�
    //ֻ����stored_ == trueʱʹ��
    void RowSeek(U32 offset){
        assert(IsValid());
        assert(IsStored());
        mysql_data_seek(res_,offset);
    }
    //���ؽ���е�����
    //��stored_ == falseʱ��������ȷ��ֵ�����Ǽ��������е�������
    U32 RowSize() const{
        assert(IsValid());
        return mysql_num_rows(res_);
    }
    //���ؽ������һ��
    CMyRow RowNext(){
        assert(IsValid());
        CMyRow ret;
        ret.row_ = mysql_fetch_row(res_);
        ret.lengths_ = mysql_fetch_lengths(res_);
        ret.size_ = FieldSize();
        return ret;
    }
    //------------Field��Ϣ------------
    //���ر��е�����
    U32 FieldSize() const{
        assert(IsValid());
        return mysql_num_fields(res_);
    }
    //����ָ���е��ֶζ�����Ϣ
    CMyFieldInfo FieldInfo(U32 index){
        assert(IsValid());
        assert(index < FieldSize());
        return mysql_fetch_field_direct(res_,index);
    }

};

//�ַ�����Ϣ
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
    //------------�������ݿ�------------
    CMySQL():conn_(0){}    //����������
    explicit CMySQL(__DZ_STRING host,__DZ_STRING user = "",
        __DZ_STRING passwd = "",__DZ_STRING database = "")  //�������ݿ�
        : conn_(0)
    {
        Connect(host,user,passwd,database);
    }
    //�ر�����
    ~CMySQL(){Close();}
    //�������ݿ�
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
    //�ж��Ƿ�������
    bool IsConnected() const{return conn_ != 0;}
    bool operator !() const{return !conn_;}
    //�����������������Ƿ�����������Ӷ�ʧ�����Զ�����������
    //������������Ƿ���Ч
    bool Ping(){
        assert(IsConnected());
        return mysql_ping(conn_) == 0;
    }
    //�ر����ӣ��ͷ�conn_��Դ
    void Close(){
        if(IsConnected()){
            mysql_close(conn_);
            conn_ = 0;
        }
    }
    //------------��ѯ------------
    //ʹ��dbָ�������ݿ��ΪĬ�����ݿ⣨��ǰ���ݿ⣩
    //�ں�����ѯ�У������ݿ⽫��δ������ȷ���ݿ����ַ��ı����õ�Ĭ�����ݿ�
    bool SelectDB(__DZ_STRING db){
        assert(IsConnected());
        return mysql_select_db(conn_,db.c_str()) == 0;
    }
    //ִ��sql���
    bool Query(__DZ_STRING query){
        assert(IsConnected());
        return !mysql_real_query(conn_,query.c_str(),query.length());
    }
    //�����ϴ�UPDATE���ĵ��������ϴ�DELETEɾ�������������ϴ�INSERT�����������
    //����-1��ʾ��ѯ����
    S32 AffectedRows() const{
        assert(IsConnected());
        my_ulonglong ret = mysql_affected_rows(conn_);
        if(ret == (my_ulonglong)-1)
            return -1;
        return S32(ret);
    }
    //���������ѯ���������
    U32 LastFieldCount() const{
        assert(IsConnected());
        return mysql_field_count(conn_);
    }
    //��������ǰ��INSERT��UPDATE���ΪAUTO_INCREMENT�����ɵ�ֵ
    U32 LastInsertID() const{
        assert(IsConnected());
        return U32(mysql_insert_id(conn_));
    }
    //------------���------------
    //����ѯ��ȫ�������ȡ���ͻ���
    //���ؽ���Ƿ�Ϊ��
    bool StoreResult(CMyResult & res){
        assert(IsConnected());
        res.Free();
        res.res_ = mysql_store_result(conn_);
        res.stored_ = true;
        return res.IsValid();
    }
    //����ʼ��������������������������ʵ�ʶ�ȡ���ͻ���
    //���ؽ���Ƿ�Ϊ��
    bool UseResult(CMyResult & res){
        assert(IsConnected());
        res.Free();
        res.res_ = mysql_use_result(conn_);
        res.stored_ = false;
        return res.IsValid();
    }
    //���ص�ǰִ�еĲ�ѯ�Ƿ���ڶ�����
    bool HasMultiResults() const{
        assert(IsConnected());
        return mysql_more_results(conn_);
    }
    //�����ڶ�����ʱ��ʹ��StoreResult�õ���һ�����
    //���ؽ���Ƿ�Ϊ��
    bool StoreNextResults(CMyResult & res){
        assert(IsConnected());
        if(mysql_next_result(conn_) == 0)
            return StoreResult(res);
        return false;
    }
    //�����ڶ�����ʱ��ʹ��UseResult�õ���һ�����
    //���ؽ���Ƿ�Ϊ��
    bool UseNextResults(CMyResult & res){
        assert(IsConnected());
        if(mysql_next_result(conn_) == 0)
            return UseResult(res);
        return false;
    }
    //------------�����뾯����Ϣ------------
    //����ִ��ǰһ��SQL������ɵĸ澯��Ŀ
    UINT WarningCount() const{
        assert(IsConnected());
        return mysql_warning_count(conn_);
    }
    //����������õ�API�����Ĵ�����Ϣ
    //���δ���ִ��󣬷��ؿ��ַ���
    __DZ_STRING ErrorMsg() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_error(conn_));
    }
    //����������õ�API�����Ĵ�����룬0��ʾδ���ִ���
    U32 ErrorNo() const{
        assert(IsConnected());
        return mysql_errno(conn_);
    }
    //------------�ַ�������------------
    //��������SQL�����ʹ�õĺϷ�SQL�ַ���
    //��Ҫת����ַ�ΪNUL(ASCII 0)����\n������\r������\������'������"�����Լ�Control-Z
    __DZ_STRING EscapeString(__DZ_STRING str) const{
        assert(IsConnected());
        __DZ_STRING ret(str.length() * 2 + 1,0);
        U32 len = mysql_real_escape_string(conn_,&ret[0],str.c_str(),str.length());
        ret.resize(len);
        return ret;
    }
    //���ַ�������ʽ�ϱ���Ϊʮ�����Ƹ�ʽ��ÿ���ַ��������Ϊ2��ʮ��������
    //�Ա�ɲ���0xvalue��X'value'��ʽ���ַ�������SQL�����
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
    //------------�ַ�����Ϣ------------
    //��ÿͻ����ַ�������Ϣ
#ifdef __HAS_MYSQL_CHARSET_INFO
    void GetCharacterSetInfo(CMyCharacterSetInfo & info) const{
        assert(IsConnected());
        mysql_get_character_set_info(conn_,&info.cs_);
    }
#endif
    //���ص�ǰ����Ĭ�ϵ��ַ�������
    __DZ_STRING GetCharacterSetName() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_character_set_name(conn_));
    }
    //����Ĭ�ϵ��ַ������ַ���csnameָ����1����Ч���ַ�������
#ifdef __HAS_MYSQL_CHARSET_INFO
    bool SetCharacterSet(__DZ_STRING csname){
        assert(IsConnected());
        return mysql_set_character_set(conn_,csname.c_str()) == 0;
    }
#endif
    //------------�汾������ͳ����Ϣ------------
    //���ر�ʾ�ͻ��˿�汾���ַ��������磺5.0.45
    static __DZ_STRING GetClientInfo(){
        return MySQLHelp::_2str(mysql_get_client_info());
    }
    //���ر�ʾ�ͻ��˿�汾������
    //ֵ�ĸ�ʽ��XYYZZ������X�����汾�ţ�YY�Ƿ�������ZZ�Ƿ��������ڵİ汾��
    //���磬ֵ40102��ʾ�ͻ��˿�İ汾��4.1.2
    static U32 GetClientVersion(){
        return mysql_get_client_version();
    }
    //���������������͵��ַ���������������������
    //����"Localhost via UNIX socket"
    __DZ_STRING GetHostInfo() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_get_host_info(conn_));
    }
    //���ص�ǰ������ʹ�õ�Э��汾
    //��ʽΪ��major_version*10000 + minor_version *100 + sub_version
    //���磺50045
    U32 GetProtoInfo() const{
        assert(IsConnected());
        return mysql_get_proto_info(conn_);
    }
    //��������ʽ���ط������İ汾��
    //���磬����5.0.12������500012
    U32 GetServerVersion() const{
        assert(IsConnected());
        return mysql_get_server_version(conn_);
    }
    //���ذ����ض���Ϣ���ַ���
    //��������Ϊ��λ����������ʱ�䣬�Լ������̵߳���Ŀ�����������ټ��ش������Լ��򿪵ı���Ŀ
    //���磺
    //Uptime: 157911  Threads: 1  Questions: 5  Slow queries: 0  Opens: 12  Flush tables: 1  Open tables: 6  Queries per second avg: 0.000
    __DZ_STRING Stat() const{
        assert(IsConnected());
        return MySQLHelp::_2str(mysql_stat(conn_));
    }
    //------------mysql�߳�------------
    //���ص�ǰ���ӵ��߳�ID
    //������Ӷ�ʧ����ʹ��Ping()�����������ӣ��߳�ID���ı�
    U32 ThreadID() const{
        assert(IsConnected());
        return mysql_thread_id(conn_);
    }
    //���������ɱ����pidָ�����̣߳������Ƿ�ɹ�
    bool Kill(U32 pid){
        assert(IsConnected());
        return mysql_kill(conn_,pid) == 0;
    }
    //------------������------------
    //�ύ��ǰ����
    bool Commit(){
        assert(IsConnected());
        return mysql_commit(conn_) == 0;
    }
    //�ع���ǰ����
    bool Rollback(){
        assert(IsConnected());
        return mysql_rollback(conn_) == 0;
    }
    //����/����autocommitģʽ
    //���������Ƿ�ɹ�
    bool SetAutoCommit(bool enable = true){
        assert(IsConnected());
        return mysql_autocommit(conn_,enable ? 1 : 0) == 0;
    }
};

NS_SERVER_END

#endif

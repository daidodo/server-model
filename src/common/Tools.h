#ifndef DOZERG_TOOLS_H_20070905
#define DOZERG_TOOLS_H_20070905

/*
    有用的类和函数
        CTriple
        HashFn
        IsReadable
        DumpHex
        DumpStr
        DumpVal
        Dump
        DumpFormat
        UnHexChar
        UnHex
        HostByteOrder
        SwapByteOrder
        Hex
        Trim
        FindFirst
        StringMatch
        StringMatchKMP
        GetTimespec
        GetTimeUs
        TimeString
        TimeStringUs
        IPv4String
        IPv4FromStr
        IPv4FromEth
        SafeCopy
        Construct
        Destroy
        DestroyArray
        New
        NewA
        Delete
        DeleteA
        SetMaxFileDescriptor
        GetMaxFileDescriptor
        GetProcessorCount
        GetPhysicalMemorySize
        GetPageSize
        ProgramName
        ExtractArg
        ToStringPtr
        toStringPtr
        ToStringBits
        ErrorMsg
        Daemon
        GetHost
        UrlEncode
        UrlDecode
        XmlEncode
        AbsFilename
        Basename
        IsTimeout
        ArraySize
        CxxDemangle
        Crc
        Md5
        IsPrime
        PrimeLess
        MEM_OFFSET
        iterator_traits
//*/

#include <string>
#include <vector>
#include <functional>           //std::unary_function
#include <utility>              //std::pair,std::make_pair
#include <endian.h>             //BYTE_ORDER,LITTLE_ENDIAN
#include <impl/Config.h>
#include <Template.h>           //CTypeTraits

NS_SERVER_BEGIN

namespace Tools
{

    //三元组,类似std::pair
    template<class T1,class T2,class T3>
    struct CTriple
    {
        typedef T1 first_type;
        typedef T2 second_type;
        typedef T3 third_type;
        first_type  first;
        second_type second;
        third_type  third;
        explicit CTriple(const first_type & f = first_type(),const second_type & s = second_type(),const third_type & t = third_type())
            : first(f)
            , second(s)
            , third(t)
        {}
    };

    //字符a是否是可读，即ASCII码属于[32,126]
    inline bool IsReadable(U8 a){
        return a > 31 && !(a & 0x80);
    }

    inline bool IsReadable(S8 a){
        return IsReadable(U8(a));
    }

    inline bool IsReadable(char a){
        return IsReadable(U8(a));
    }

    //得到数据v的16进制字符串表示
    //默认格式示例："abc" = (3)61 62 63
    //sep为分隔符,默认' '
    //hasLen表示是否有前面的数据长度"(3)"
    std::string DumpHex(const char * v,size_t sz,char sep = ' ',bool hasLen = true);

    inline std::string DumpHex(const signed char * v,size_t sz,char sep = ' ',bool hasLen = true){
        return DumpHex((const char *)v,sz,sep,hasLen);
    }

    inline std::string DumpHex(const unsigned char * v,size_t sz,char sep = ' ',bool hasLen = true){
        return DumpHex((const char *)v,sz,sep,hasLen);
    }

    inline std::string DumpHex(const std::vector<char> & v,char sep = ' ',bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpHex(&v[0],v.size(),sep,hasLen);
    }

    inline std::string DumpHex(std::string v,char sep = ' ',bool hasLen = true){
        return DumpHex(v.c_str(),v.length(),sep,hasLen);
    }

    //得到数据v的可打印形式，非可读字符用replace代替
    //默认格式示例："a\t\0bc" = (5)a..bc
    //replace为替代符,默认'.'
    //hasLen表示是否有前面的数据长度"(5)"
    std::string DumpStr(const char * v,size_t sz,char replace = '.',bool hasLen = true);

    inline std::string DumpStr(const signed char * v,size_t sz,char replace = '.',bool hasLen = true){
        return DumpStr((const char *)v,sz,replace,hasLen);
    }

    inline std::string DumpStr(const unsigned char * v,size_t sz,char replace = '.',bool hasLen = true){
        return DumpStr((const char *)v,sz,replace,hasLen);
    }

    inline std::string DumpStr(const std::vector<char> & v,char replace = '.',bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpStr(&v[0],v.size(),replace,hasLen);
    }

    inline std::string DumpStr(std::string str,char replace = '.',bool hasLen = true){
        return DumpStr(str.c_str(),str.length(),replace,hasLen);
    }

    //得到数据v的可打印形式，不可打印字符用base进制数值表示
    //默认格式示例："a\t\223bc" = (5)a\t\223bc
    //base取值为8，16
    //hasLen表示是否有前面的数据长度"(5)"
    std::string DumpVal(const char * v,size_t sz,int base = 8,bool hasLen = true);

    inline std::string DumpVal(const signed char * v,size_t sz,char base = 8,bool hasLen = true){
        return DumpVal((const char *)v,sz,base,hasLen);
    }

    inline std::string DumpVal(const unsigned char * v,size_t sz,char base = 8,bool hasLen = true){
        return DumpVal((const char *)v,sz,base,hasLen);
    }

    inline std::string DumpVal(const std::vector<char> & v,char base = 8,bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpVal(&v[0],v.size(),base,hasLen);
    }

    inline std::string DumpVal(std::string str,char base = 8,bool hasLen = true){
        return DumpVal(str.c_str(),str.length(),base,hasLen);
    }

    //得到数据v的可打印形式，自动选择DumpHex，DumpStr或DumpVal
    //show_sz表示显示出来的数据长度，剩余的数据用"..."代替
    //hasLen表示是否有前面的数据长度
    std::string Dump(const char * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true);

    inline std::string Dump(const signed char * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump((const char *)v,sz,show_sz,hasLen);
    }

    inline std::string Dump(const unsigned char * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump((const char *)v,sz,show_sz,hasLen);
    }

    inline std::string Dump(const std::vector<char> & v,size_t show_sz = size_t(-1),bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : Dump(&v[0],v.size(),show_sz,hasLen);
    }

    inline std::string Dump(std::string str,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump(str.c_str(),str.length(),show_sz,hasLen);
    }

    //显示字符串的16进制和可视形式
    std::string DumpFormat(const char * v, size_t sz);
    
    //得到16进制字符a表示的10进制数值，错误时返回-1
    inline int UnHexChar(char a){
        if(a >= '0' && a <= '9')
            return a - '0';
        else if(a >= 'a' && a <= 'f')
            return a - 'a' + 0xa;
        else if(a >= 'A' && a <= 'F')
            return a - 'A' + 0xA;
        else
            return -1;
    }

    inline int UnHexChar(S8 a){
        return UnHexChar(char(a));
    }

    inline int UnHexChar(U8 a){
        return UnHexChar(char(a));
    }

    //把数据的16进制还原成数据本身
    std::string UnHex(const char * v,size_t sz);

    inline std::string UnHex(const U8 * v,size_t sz){
        return UnHex((const char *)v,sz);
    }

    inline std::string UnHex(const S8 * v,size_t sz){
        return UnHex((const char *)v,sz);
    }

    inline std::string UnHex(const std::vector<char> & v){
        return UnHex(&v[0],v.size());
    }

    inline std::string UnHex(std::string v){
        return UnHex(v.c_str(),v.length());
    }

    //得到主机的字节序
    //return:
    //  true    little endian
    //  false   big endian(网络序)
    inline bool HostByteOrder(){
#ifdef BYTE_ORDER
#   if BYTE_ORDER == LITTLE_ENDIAN
        return true;
#   else
        return false;
#   endif
#else
        int t = 1;
        char * p = reinterpret_cast<char *>(&t);
        return (*p == 1);
#endif
    }

    //改变v的byte order.要求T是原始数据类型
    template<typename T>
    T SwapByteOrder(T v){
        return CByteOrderTraits<T,sizeof(T)>::Swap(v);
    }

    //得到v的16进制表示
    //由于字节序原因,可能与cout<<hex<<v的结果不同
    template<typename T>
    std::string Hex(T v){
        const char * p = reinterpret_cast<const char *>(&v);
        return DumpHex(p,sizeof v);
    }

    //去除str头尾的空白符
    std::string Trim(std::string str);
    inline std::string Trim(const char * str,size_t len){
        return Trim(std::string(str,len));
    }

    //类似于string的find_first_of
    //str和len为原字符串和长度
    //ch为要查找的字符,off为查找起始偏移
    size_t FindFirst(const char * str,size_t len,char ch,size_t off = 0);

    //在src串里匹配target串
    //找到返回匹配串的起始位置
    //未找到返回-1
    ssize_t StringMatch(const char * src,size_t srclen,const char * target,size_t tarlen);

    //在src串里匹配target串，使用KMP算法
    //找到返回匹配串的起始位置
    //未找到返回-1
    ssize_t StringMatchKmp(const char * src,size_t srclen,const char * target,size_t tarlen);

    //某些函数需要制定到期时间tp.
    //根据timeMs毫秒生成到期时间
    void GetTimespec(U32 timeMs,struct timespec & ts);

    //得到微秒级的当前时间 + elapse(微秒)
    U64 GetTimeUs(U64 elapse = 0);

    //秒级别的时间字符串,格式设置参考strftime函数
    std::string TimeString(U32 timeS,std::string format = "%y-%m-%d %H:%M:%S");

    //微秒级别的时间字符串,格式设置参考strftime函数
    std::string TimeStringUs(U64 timeMs,std::string format = "%y-%m-%d %H:%M:%S");

    //IPv4地址与字符串的转换
    std::string IPv4String(U32 ip,bool hostByteOrder = true);

    U32 IPv4FromStr(std::string ip,bool hostByteOrder = true);

    U32 IPv4FromEth(const std::string & eth, bool hostByteOrder = true);

    //把[src_first,src_last)的内容copy到[dst_first,dst_first + dst_size)区间
    //return实际copy后,元素的src和dst尾标杆
    template<class InputIterator,class OutputIterator,class Size>
    std::pair<InputIterator,OutputIterator> SafeCopy
        (InputIterator src_first,InputIterator src_last,OutputIterator dst_first,Size dst_size){
            for(;dst_size > 0 && src_first != src_last;--dst_size,++src_first,++dst_first)
                *dst_first = *src_first;
            return std::make_pair<InputIterator,OutputIterator>(src_first,dst_first);
    }

    //封装构造函数,使内存分配与对象构造分离
    template<class T1,class T2>
    void Construct(T1 * p,const T2 & v){
        new (p) T1(v);
    }

    //使用a销毁p，dtor表示是否需要调用p的析构函数
    //主要作用是把p重置为0
    template<class T,class A>
    void Destroy(T *& p,A a,bool dtor = true) __DZ_NOTHROW{
        typedef char (&dummy)[sizeof(T)];   //保证T类型的完整性
        if(p){
            if(dtor)
                a.destroy(p);
            a.deallocate(p,1);
            p = 0;
        }
    }
    template<class T,class A>
    void DestroyArray(T *& p,size_t sz,A a,bool dtor = true) __DZ_NOTHROW{
        typedef char (&dummy)[sizeof(T)];   //保证T类型的完整性
        if(p){
            if(dtor){
                for(size_t i = 0;i < sz;++i)
                    a.destroy(p + i);
            }
            a.deallocate(p,sz);
            p = 0;
        }
    }

    //封装new和delete，方便使用allocator
    template<class T>
    T * New(){
        T * ret = std::allocator<T>().allocate(1);
        return new (ret) T;
    }

    template<class T1,class T2>
    T1 * New(const T2 & v){
        T1 * ret = std::allocator<T1>().allocate(1);
        return new (ret) T1(v);
    }

    template<class T,class A>
    T * NewA(A a){
        T * ret = a.allocate(1);
        return new (ret) T;
    }

    template<class T1,class T2,class A>
    T1 * NewA(const T2 & v,A a){
        T1 * ret = a.allocate(1);
        return new (ret) T1(v);
    }

    template<class T>
    void Delete(T *& p){
        Destroy(p,std::allocator<T>());
    }

    template<class T,class A>
    void DeleteA(T *& p,A a){
        Destroy(p,a);
    }

    //设置进程允许打开的最大文件数
    bool SetMaxFileDescriptor(U32 numfiles);

    //获得进程允许打开的最大文件数,返回0表示获取失败
    U32 GetMaxFileDescriptor();

    //获得计算机处理器个数,返回-1表示获取失败
    int GetProcessorCount();

    //获取计算机物理内存大小(bytes)
    U64 GetPhysicalMemorySize();

    //获取系统页表大小(bytes)
    size_t GetPageSize();

    //得到程序的文件名，去掉目录部分
    const char * ProgramName(const char * argstr);
    inline std::string ProgramName(std::string argstr){
        return argstr.substr(argstr.find_last_of('/') + 1);
    }

    //解析命令行选项，argstr为待解析命令行，pattern为需要匹配的前几个字符，result为去掉pattern后argstr剩下的参数部分
    //返回argstr是否匹配pattern
    //例如:
    //argstr = "-file=example.txt"
    //当匹配 pattern = "-file=" 时,返回true,且得到 result = "example.txt"
    //当匹配 pattern = "-logfile="时,返回false
    bool ExtractArg(const char * argstr,const char * pattern,const char *& result);

    //调用对象指针p的ToString()函数时，进行安全检查
    template<class Ptr>
    std::string ToStringPtr(const Ptr & p){
        return (p ? p->ToString() : "NULL");
    }

    //调用对象指针p的ToString()函数时，进行安全检查
    template<class Ptr>
    std::string toStringPtr(const Ptr & p){
        return (p ? p->toString() : "NULL");
    }

    //将U32类型的v及每个bit的name显示出来
    //name: 指定每个bit的名称，为0时不显示
    //name_len: name数组的长度
    //return示例: 3(IN | OUT)
    std::string ToStringBits(U32 v, const char * const * name, size_t name_len);

    //得到错误码error_no对应的系统错误信息
    std::string ErrorMsg(int error_no);

    //程序进到后台运行
    //closeFiles: 是否关闭所有打开的文件(除了0,1,2)
    void Daemon(bool closeFiles = false);

    //从url里取出host
    std::string GetHost(const std::string & url);

    //url转码处理
    std::string UrlEncode(const std::string & url);
    std::string UrlDecode(const std::string & url);

    //xml转码处理
    std::string XmlEncode(const std::string & val);

    //如果fname不是绝对路径，补全成绝对路径后返回
    std::string AbsFilename(const std::string & fname);

    //去掉fname里的目录部分，只留文件名
    std::string Basename(const std::string & fname);

    //判断是否超时
    //curtime: 当前时间
    //oldtime: 需要判断的时间,0表示永不超时
    //interval: 超时间隔,<0表示永不超时
    //jump: 允许的时间跳变
    inline bool IsTimeout(time_t curtime, time_t oldtime, int interval, int jump = 60);

    //返回数组的元素个数
    template<class T, size_t N>
    size_t ArraySize(T (&)[N]){return N;}

    //将std::typeinfo::name()的返回值转换成可读的类型名
    std::string CxxDemangle(const char * name);

    //计算buf的crc
    template<typename Int>
    Int Crc(Int init, const char * buf, size_t sz){
        const Int SIGN = (Int(1) << (CTypeTraits<Int>::MAX_BITS - 1));
        Int add = 0;
        for(size_t i = 0;i < sz;++i, init = (init << 1) + add)
            add = (init & SIGN ? 1 : 0) + buf[i];
        return init;
    }

    template<typename Int>
    Int Crc(Int init, const std::string & buf){
        return Crc(init, &buf[0], buf.size());
    }

    template<typename Int>
    Int Crc(Int init, const std::vector<char> & buf){
        return Crc(init, &buf[0], buf.size());
    }

    //计算buf的md5
    //TODO
    std::string Md5(const char * buf, size_t sz);

    inline std::string Md5(const std::string & buf){
        return Md5(&buf[0], buf.size());
    }

    inline std::string Md5(const std::vector<char> & buf){
        return Md5(&buf[0], buf.size());
    }

    //返回v是否素数
    bool IsPrime(int v);

    //返回小于等于v的最大素数
    //如果没有，返回0
    int PrimeLess(int v);

    //specialization for integer types
    //区分iterator类型与数值类型,用于下面的iterator_traits
    struct CIntegerTypeTag{};  //integer type tag

}//namespace Tools

NS_SERVER_END

#define ITERATOR_TRAITS_FOR_INTEGER_TYPE(TYPE)  \
    template<>struct iterator_traits< TYPE >{   \
    typedef NS_SERVER::Tools::CIntegerTypeTag iterator_category;}
namespace std{
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(bool);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(char);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(signed char);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(unsigned char);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(wchar_t);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(short);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(unsigned short);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(int);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(unsigned int);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(long);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(unsigned long);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(long long);
    ITERATOR_TRAITS_FOR_INTEGER_TYPE(unsigned long long);
//NOTE: volatile and const are not considered.
}//namespace std

#undef ITERATOR_TRAITS_FOR_INTEGER_TYPE

#endif


#ifndef DOZERG_TOOLS_H_20070905
#define DOZERG_TOOLS_H_20070905

/*
    有用的类和函数
        CInterTypeTag
        CIdentity
        CTriple
        CTypeSelector
        HashFn
        IsReadable
        DumpHex
        DumpStr
        DumpVal
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
        ProgramName
        ExtractArg
        ToStringPtr
        ErrorMsg
        MEM_OFFSET
        iterator_traits
//*/

#include <functional>   //std::unary_function
#include <string>
#include <vector>
#include <utility>      //std::pair,std::make_pair
#include <endian.h>     //BYTE_ORDER,LITTLE_ENDIAN
#include <iomanip>      //std::setw,std::setfill
#include <common/impl/Tools_impl.h>
#include <common/impl/Alloc.h>

NS_SERVER_BEGIN

namespace Tools
{
    template<class T>
    struct CIdentity : public std::unary_function<T,T> {
        T & operator()(T & v) const{return v;}
        const T & operator()(const T & v) const{return v;}
    };

    template<class Pair>
    struct CSelect1st:public std::unary_function<Pair,typename Pair::first_type> {
        typename Pair::first_type & operator()(Pair & p) const{
            return p.first;
        }
        const typename Pair::first_type & operator()(const Pair & p) const {
            return p.first;
        }
    };

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

    //类型选择器
    template<class T1,class T2,bool Sel>
    struct CTypeSelector{
        typedef T1  RType;
    };
    template<class T1,class T2>
    struct CTypeSelector<T1,T2,false>{
        typedef T2  RType;
    };
    
    //hash函数集合
    inline size_t __stl_hash_string(const char * s){
        size_t ret = 0; 
        for(;s && *s;++s)
            ret = 5 * ret + *s;
        return ret;
    }
    inline size_t __stl_hash_string(const char * s,size_t sz){
        size_t ret = 0; 
        for(size_t i = 0;i < sz;++s,++i)
            ret = 5 * ret + *s;
        return ret;
    }
    template<class Key>struct HashFn{
        size_t operator()(const Key & v) const{
            return v.HashFn();
        }
    };
#define TEMPLATE_INSTANCE_FOR_TYPE(TYPE,HASH)   template<>struct HashFn<TYPE>{  \
    size_t operator()(TYPE v) const{return (HASH);}}
    TEMPLATE_INSTANCE_FOR_TYPE(char *,__stl_hash_string(v));
    TEMPLATE_INSTANCE_FOR_TYPE(const char *,__stl_hash_string(v));
    TEMPLATE_INSTANCE_FOR_TYPE(signed char *,__stl_hash_string((const char *)v));
    TEMPLATE_INSTANCE_FOR_TYPE(const signed char *,__stl_hash_string((const char *)v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned char *,__stl_hash_string((const char *)v));
    TEMPLATE_INSTANCE_FOR_TYPE(const unsigned char *,__stl_hash_string((const char *)v));
    TEMPLATE_INSTANCE_FOR_TYPE(char,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(signed char,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned char,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(signed short,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned short,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(signed int,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned int,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(signed long,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned long,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(__DZ_STRING,__stl_hash_string(v.c_str(),v.length()));
#undef TEMPLATE_INSTANCE_FOR_TYPE

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
    __DZ_STRING DumpHex(const char * v,size_t sz,char sep = ' ',bool hasLen = true);

    inline __DZ_STRING DumpHex(const U8 * v,size_t sz,char sep = ' ',bool hasLen = true){
        return DumpHex((const char *)v,sz,sep,hasLen);
    }

    inline __DZ_STRING DumpHex(const S8 * v,size_t sz,char sep = ' ',bool hasLen = true){
        return DumpHex((const char *)v,sz,sep,hasLen);
    }

    inline __DZ_STRING DumpHex(const __DZ_VECTOR(char) & v,char sep = ' ',bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpHex(&v[0],v.size(),sep,hasLen);
    }

    inline __DZ_STRING DumpHex(__DZ_STRING v,char sep = ' ',bool hasLen = true){
        return DumpHex(v.c_str(),v.length(),sep,hasLen);
    }

    //得到数据v的可打印形式，非可读字符用replace代替
    //默认格式示例："a\t\0bc" = (5)a..bc
    //replace为替代符,默认'.'
    //hasLen表示是否有前面的数据长度"(5)"
    __DZ_STRING DumpStr(const char * v,size_t sz,char replace = '.',bool hasLen = true);

    inline __DZ_STRING DumpStr(const U8 * v,size_t sz,char replace = '.',bool hasLen = true){
        return DumpStr((const char *)v,sz,replace,hasLen);
    }

    inline __DZ_STRING DumpStr(const S8 * v,size_t sz,char replace = '.',bool hasLen = true){
        return DumpStr((const char *)v,sz,replace,hasLen);
    }

    inline __DZ_STRING DumpStr(const __DZ_VECTOR(char) & v,char replace = '.',bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpStr(&v[0],v.size(),replace,hasLen);
    }

    inline __DZ_STRING DumpStr(__DZ_STRING str,char replace = '.',bool hasLen = true){
        return DumpStr(str.c_str(),str.length(),replace,hasLen);
    }

    //得到数据v的可打印形式，不可打印字符用base进制数值表示
    //默认格式示例："a\t\223bc" = (5)a\t\223bc
    //base取值为8，16
    //hasLen表示是否有前面的数据长度"(5)"
    __DZ_STRING DumpVal(const char * v,size_t sz,int base = 8,bool hasLen = true);

    inline __DZ_STRING DumpVal(const U8 * v,size_t sz,char base = 8,bool hasLen = true){
        return DumpVal((const char *)v,sz,base,hasLen);
    }

    inline __DZ_STRING DumpVal(const S8 * v,size_t sz,char base = 8,bool hasLen = true){
        return DumpVal((const char *)v,sz,base,hasLen);
    }

    inline __DZ_STRING DumpVal(const __DZ_VECTOR(char) & v,char base = 8,bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : DumpVal(&v[0],v.size(),base,hasLen);
    }

    inline __DZ_STRING DumpVal(__DZ_STRING str,char base = 8,bool hasLen = true){
        return DumpVal(str.c_str(),str.length(),base,hasLen);
    }

    //得到数据v的可打印形式，自动选择DumpHex，DumpStr或DumpVal
    //show_sz表示显示出来的数据长度，剩余的数据用"..."代替
    //hasLen表示是否有前面的数据长度
    __DZ_STRING Dump(const char * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true);

    inline __DZ_STRING Dump(const U8 * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump((const char *)v,sz,show_sz,hasLen);
    }

    inline __DZ_STRING Dump(const S8 * v,size_t sz,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump((const char *)v,sz,show_sz,hasLen);
    }

    inline __DZ_STRING Dump(const __DZ_VECTOR(char) & v,size_t show_sz = size_t(-1),bool hasLen = true){
        return v.empty() ? (hasLen ? "(0)" : "") : Dump(&v[0],v.size(),show_sz,hasLen);
    }

    inline __DZ_STRING Dump(__DZ_STRING str,size_t show_sz = size_t(-1),bool hasLen = true){
        return Dump(str.c_str(),str.length(),show_sz,hasLen);
    }

    //打印出数据的16进制和内容的对比结果
    __DZ_STRING DumpFormat(const char * v,size_t sz);

    inline __DZ_STRING DumpFormat(const U8 * v,size_t sz){
        return DumpFormat((const char *)v,sz);
    }

    inline __DZ_STRING DumpFormat(const S8 * v,size_t sz){
        return DumpFormat((const char *)v,sz);
    }

    inline __DZ_STRING DumpFormat(const __DZ_VECTOR(char) & v){
        return DumpFormat(&v[0],v.size());
    }

    inline __DZ_STRING DumpFormat(const __DZ_STRING & v){
        return DumpFormat(&v[0],v.size());
    }

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
    __DZ_STRING UnHex(const char * v,size_t sz);

    inline __DZ_STRING UnHex(const U8 * v,size_t sz){
        return UnHex((const char *)v,sz);
    }

    inline __DZ_STRING UnHex(const S8 * v,size_t sz){
        return UnHex((const char *)v,sz);
    }

    inline __DZ_STRING UnHex(const __DZ_VECTOR(char) & v){
        return UnHex(&v[0],v.size());
    }

    inline __DZ_STRING UnHex(__DZ_STRING v){
        return UnHex(v.c_str(),v.length());
    }

    //得到主机的字节序,返回little endian(true)或big endian(false)
    inline bool HostByteOrder()
    {
#ifdef BYTE_ORDER
#   if BYTE_ORDER == LITTLE_ENDIAN
        return true;
#   else
        return false;
#   endif
#else
        int t = 1;
        char * p = reinterpret_cast<char *>(&t);
        return (*p == 0);
#endif
    }
    
    //改变v的byte order.要求T是原始数据类型
    template<typename T>
    T SwapByteOrder(T v){
        return NS_IMPL::CByteOrderTraits<T,sizeof(T)>::Swap(v);
    }

    //得到v的16进制表示
    //由于字节序原因,可能与cout<<hex<<v的结果不同
    template<typename T>
    __DZ_STRING Hex(T v){
        const char * p = reinterpret_cast<const char *>(&v);
        return DumpHex(p,sizeof v);
    }

    //去除str头尾的空白符
    __DZ_STRING Trim(__DZ_STRING str);
    inline __DZ_STRING Trim(const char * str,size_t len){
        return Trim(__DZ_STRING(str,len));
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
    ssize_t StringMatchKMP(const char * src,size_t srclen,const char * target,size_t tarlen);

    //某些函数需要制定到期时间tp.
    //根据timeMs毫秒生成到期时间
    void GetTimespec(U32 timeMs,struct timespec & ts);

    //得到微秒级的当前时间 + elapse(微秒)
    U64 GetTimeUs(U64 elapse = 0);

    //秒级别的时间字符串,格式设置参考strftime函数
    __DZ_STRING TimeString(U32 timeS,__DZ_STRING format = "%y-%m-%d %H:%M:%S");

    //微秒级别的时间字符串,格式设置参考strftime函数
    __DZ_STRING TimeStringUs(U64 timeMs,__DZ_STRING format = "%y-%m-%d %H:%M:%S");

    //把IPv4地址转换成字符串表示
    __DZ_STRING IPv4String(U32 ip,bool hostByteOrder = true);

    U32 IPv4FromStr(__DZ_STRING ip,bool hostByteOrder = true);

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
        T * ret = __DZ_ALLOC<T>().allocate(1);
        return new (ret) T;
    }

    template<class T1,class T2>
    T1 * New(const T2 & v){
        T1 * ret = __DZ_ALLOC<T1>().allocate(1);
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
        Destroy(p,__DZ_ALLOC<T>());
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

    //得到程序的文件名，去掉目录部分
    const char * ProgramName(const char * argstr);
    inline __DZ_STRING ProgramName(__DZ_STRING argstr){
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
    __DZ_STRING ToStringPtr(const Ptr & p){
        return (p ? p->ToString() : "0x0");
    }

    //得到错误码error_no对应的系统错误信息
    __DZ_STRING ErrorMsg(int error_no);

    //求成员变量MEMBER在类TYPE中的偏移位置
#define MEM_OFFSET(TYPE,MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

    //specialization for integer types
    //区分iterator类型与数值类型,用于下面的iterator_traits
    struct CInterTypeTag{};  //integer type tag

}//namespace Tools

NS_SERVER_END

#define ITERATOR_TRAITS_FOR_INTEGER_TYPE(TYPE)  \
    template<>struct iterator_traits< TYPE >{   \
    typedef NS_SERVER::Tools::CInterTypeTag iterator_category;}
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

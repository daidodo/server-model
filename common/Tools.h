#ifndef DOZERG_TOOLS_H_20070905
#define DOZERG_TOOLS_H_20070905

/*
    ���õ���ͺ���
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
        Daemon
        GetHost
        UrlEncode
        UrlDecode
        MEM_OFFSET
        iterator_traits
//*/

#include <common/impl/Config.h>
#include <functional>   //std::unary_function
#include <string>
#include <vector>
#include <utility>      //std::pair,std::make_pair
#include <endian.h>     //BYTE_ORDER,LITTLE_ENDIAN
#include <common/impl/Tools_impl.h>

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

    //��Ԫ��,����std::pair
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

    //����ѡ����
    template<class T1,class T2,bool Sel>
    struct CTypeSelector{
        typedef T1  RType;
    };
    template<class T1,class T2>
    struct CTypeSelector<T1,T2,false>{
        typedef T2  RType;
    };

    //hash��������
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
    TEMPLATE_INSTANCE_FOR_TYPE(signed long long,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(unsigned long long,size_t(v));
    TEMPLATE_INSTANCE_FOR_TYPE(std::string,__stl_hash_string(v.c_str(),v.length()));
#undef TEMPLATE_INSTANCE_FOR_TYPE

    //�ַ�a�Ƿ��ǿɶ�����ASCII������[32,126]
    inline bool IsReadable(U8 a){
        return a > 31 && !(a & 0x80);
    }

    inline bool IsReadable(S8 a){
        return IsReadable(U8(a));
    }

    inline bool IsReadable(char a){
        return IsReadable(U8(a));
    }

    //�õ�����v��16�����ַ�����ʾ
    //Ĭ�ϸ�ʽʾ����"abc" = (3)61 62 63
    //sepΪ�ָ���,Ĭ��' '
    //hasLen��ʾ�Ƿ���ǰ������ݳ���"(3)"
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

    //�õ�����v�Ŀɴ�ӡ��ʽ���ǿɶ��ַ���replace����
    //Ĭ�ϸ�ʽʾ����"a\t\0bc" = (5)a..bc
    //replaceΪ�����,Ĭ��'.'
    //hasLen��ʾ�Ƿ���ǰ������ݳ���"(5)"
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

    //�õ�����v�Ŀɴ�ӡ��ʽ�����ɴ�ӡ�ַ���base������ֵ��ʾ
    //Ĭ�ϸ�ʽʾ����"a\t\223bc" = (5)a\t\223bc
    //baseȡֵΪ8��16
    //hasLen��ʾ�Ƿ���ǰ������ݳ���"(5)"
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

    //�õ�����v�Ŀɴ�ӡ��ʽ���Զ�ѡ��DumpHex��DumpStr��DumpVal
    //show_sz��ʾ��ʾ���������ݳ��ȣ�ʣ���������"..."����
    //hasLen��ʾ�Ƿ���ǰ������ݳ���
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

    //�õ�16�����ַ�a��ʾ��10������ֵ������ʱ����-1
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

    //�����ݵ�16���ƻ�ԭ�����ݱ���
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

    //�õ��������ֽ���,����little endian(true)��big endian(false)
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

    //�ı�v��byte order.Ҫ��T��ԭʼ��������
    template<typename T>
    T SwapByteOrder(T v){
        return NS_IMPL::CByteOrderTraits<T,sizeof(T)>::Swap(v);
    }

    //�õ�v��16���Ʊ�ʾ
    //�����ֽ���ԭ��,������cout<<hex<<v�Ľ����ͬ
    template<typename T>
    std::string Hex(T v){
        const char * p = reinterpret_cast<const char *>(&v);
        return DumpHex(p,sizeof v);
    }

    //ȥ��strͷβ�Ŀհ׷�
    std::string Trim(std::string str);
    inline std::string Trim(const char * str,size_t len){
        return Trim(std::string(str,len));
    }

    //������string��find_first_of
    //str��lenΪԭ�ַ����ͳ���
    //chΪҪ���ҵ��ַ�,offΪ������ʼƫ��
    size_t FindFirst(const char * str,size_t len,char ch,size_t off = 0);

    //��src����ƥ��target��
    //�ҵ�����ƥ�䴮����ʼλ��
    //δ�ҵ�����-1
    ssize_t StringMatch(const char * src,size_t srclen,const char * target,size_t tarlen);

    //��src����ƥ��target����ʹ��KMP�㷨
    //�ҵ�����ƥ�䴮����ʼλ��
    //δ�ҵ�����-1
    ssize_t StringMatchKMP(const char * src,size_t srclen,const char * target,size_t tarlen);

    //ĳЩ������Ҫ�ƶ�����ʱ��tp.
    //����timeMs�������ɵ���ʱ��
    void GetTimespec(U32 timeMs,struct timespec & ts);

    //�õ�΢�뼶�ĵ�ǰʱ�� + elapse(΢��)
    U64 GetTimeUs(U64 elapse = 0);

    //�뼶���ʱ���ַ���,��ʽ���òο�strftime����
    std::string TimeString(U32 timeS,std::string format = "%y-%m-%d %H:%M:%S");

    //΢�뼶���ʱ���ַ���,��ʽ���òο�strftime����
    std::string TimeStringUs(U64 timeMs,std::string format = "%y-%m-%d %H:%M:%S");

    //��IPv4��ַת�����ַ�����ʾ
    std::string IPv4String(U32 ip,bool hostByteOrder = true);

    U32 IPv4FromStr(std::string ip,bool hostByteOrder = true);

    //��[src_first,src_last)������copy��[dst_first,dst_first + dst_size)����
    //returnʵ��copy��,Ԫ�ص�src��dstβ���
    template<class InputIterator,class OutputIterator,class Size>
    std::pair<InputIterator,OutputIterator> SafeCopy
        (InputIterator src_first,InputIterator src_last,OutputIterator dst_first,Size dst_size){
            for(;dst_size > 0 && src_first != src_last;--dst_size,++src_first,++dst_first)
                *dst_first = *src_first;
            return std::make_pair<InputIterator,OutputIterator>(src_first,dst_first);
    }

    //��װ���캯��,ʹ�ڴ��������������
    template<class T1,class T2>
    void Construct(T1 * p,const T2 & v){
        new (p) T1(v);
    }

    //ʹ��a����p��dtor��ʾ�Ƿ���Ҫ����p����������
    //��Ҫ�����ǰ�p����Ϊ0
    template<class T,class A>
    void Destroy(T *& p,A a,bool dtor = true) __DZ_NOTHROW{
        typedef char (&dummy)[sizeof(T)];   //��֤T���͵�������
        if(p){
            if(dtor)
                a.destroy(p);
            a.deallocate(p,1);
            p = 0;
        }
    }
    template<class T,class A>
    void DestroyArray(T *& p,size_t sz,A a,bool dtor = true) __DZ_NOTHROW{
        typedef char (&dummy)[sizeof(T)];   //��֤T���͵�������
        if(p){
            if(dtor){
                for(size_t i = 0;i < sz;++i)
                    a.destroy(p + i);
            }
            a.deallocate(p,sz);
            p = 0;
        }
    }

    //��װnew��delete������ʹ��allocator
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

    //���ý�������򿪵�����ļ���
    bool SetMaxFileDescriptor(U32 numfiles);

    //��ý�������򿪵�����ļ���,����0��ʾ��ȡʧ��
    U32 GetMaxFileDescriptor();

    //��ü��������������,����-1��ʾ��ȡʧ��
    int GetProcessorCount();

    //�õ�������ļ�����ȥ��Ŀ¼����
    const char * ProgramName(const char * argstr);
    inline std::string ProgramName(std::string argstr){
        return argstr.substr(argstr.find_last_of('/') + 1);
    }

    //����������ѡ�argstrΪ�����������У�patternΪ��Ҫƥ���ǰ�����ַ���resultΪȥ��pattern��argstrʣ�µĲ�������
    //����argstr�Ƿ�ƥ��pattern
    //����:
    //argstr = "-file=example.txt"
    //��ƥ�� pattern = "-file=" ʱ,����true,�ҵõ� result = "example.txt"
    //��ƥ�� pattern = "-logfile="ʱ,����false
    bool ExtractArg(const char * argstr,const char * pattern,const char *& result);

    //���ö���ָ��p��ToString()����ʱ�����а�ȫ���
    template<class Ptr>
    std::string ToStringPtr(const Ptr & p){
        return (p ? p->ToString() : "NULL");
    }

    //�õ�������error_no��Ӧ��ϵͳ������Ϣ
    std::string ErrorMsg(int error_no);

    //���������̨����
    //closeFiles: �Ƿ�ر����д򿪵��ļ�(����0,1,2)
    void Daemon(bool closeFiles = false);

    //��url��ȡ��host
    std::string GetHost(const std::string & url);

    //urlת�봦��
    std::string UrlEncode(const std::string & url);
    std::string UrlDecode(const std::string & url);

    //specialization for integer types
    //����iterator��������ֵ����,���������iterator_traits
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

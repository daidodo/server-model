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
    template<class Key>struct HashFn{};
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

    //�õ�����v�Ŀɴ�ӡ��ʽ���ǿɶ��ַ���replace����
    //Ĭ�ϸ�ʽʾ����"a\t\0bc" = (5)a..bc
    //replaceΪ�����,Ĭ��'.'
    //hasLen��ʾ�Ƿ���ǰ������ݳ���"(5)"
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

    //�õ�����v�Ŀɴ�ӡ��ʽ�����ɴ�ӡ�ַ���base������ֵ��ʾ
    //Ĭ�ϸ�ʽʾ����"a\t\223bc" = (5)a\t\223bc
    //baseȡֵΪ8��16
    //hasLen��ʾ�Ƿ���ǰ������ݳ���"(5)"
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

    //�õ�����v�Ŀɴ�ӡ��ʽ���Զ�ѡ��DumpHex��DumpStr��DumpVal
    //show_sz��ʾ��ʾ���������ݳ��ȣ�ʣ���������"..."����
    //hasLen��ʾ�Ƿ���ǰ������ݳ���
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
    __DZ_STRING Hex(T v){
        const char * p = reinterpret_cast<const char *>(&v);
        return DumpHex(p,sizeof v);
    }

    //ȥ��strͷβ�Ŀհ׷�
    __DZ_STRING Trim(__DZ_STRING str);
    inline __DZ_STRING Trim(const char * str,size_t len){
        return Trim(__DZ_STRING(str,len));
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

    //�õ�΢�뼶�ĵ�ǰʱ�� + elapse(����)
    U64 GetTimeUs(U64 elapse = 0);

    //�뼶���ʱ���ַ���,��ʽ���òο�strftime����
    __DZ_STRING TimeString(U32 timeS,__DZ_STRING format = "%y-%m-%d %H:%M:%S");

    //΢�뼶���ʱ���ַ���,��ʽ���òο�strftime����
    __DZ_STRING TimeStringUs(U64 timeMs,__DZ_STRING format = "%y-%m-%d %H:%M:%S");

    //��IPv4��ַת�����ַ�����ʾ
    __DZ_STRING IPv4String(U32 ip,bool hostByteOrder = true);

    U32 IPv4FromStr(__DZ_STRING ip,bool hostByteOrder = true);

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

    //���ý�������򿪵�����ļ���
    bool SetMaxFileDescriptor(U32 numfiles);

    //��ý�������򿪵�����ļ���,����0��ʾ��ȡʧ��
    U32 GetMaxFileDescriptor();

    //��ü��������������,����-1��ʾ��ȡʧ��
    int GetProcessorCount();

    //�õ�������ļ�����ȥ��Ŀ¼����
    const char * ProgramName(const char * argstr);
    inline __DZ_STRING ProgramName(__DZ_STRING argstr){
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
    __DZ_STRING ToStringPtr(const Ptr & p){
        return (p ? p->ToString() : "0x0");
    }

    //�õ�������error_no��Ӧ��ϵͳ������Ϣ
    __DZ_STRING ErrorMsg(int error_no);

    //���Ա����MEMBER����TYPE�е�ƫ��λ��
#define MEM_OFFSET(TYPE,MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

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

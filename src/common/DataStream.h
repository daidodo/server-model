#ifndef DOZERG_DATA_STREAM_H_20070905
#define DOZERG_DATA_STREAM_H_20070905

/*
    封装数据流的读取和写入
    注意测试operator !(), 在错误状态下, 所有读写数据操作都会无效
    类型:
        CInByteStream       以字节为单位的输入流
        COutByteStreamBasic 以任意buf为底层的字节输出流
        COutByteStream      以std::string为底层buf的字节输出流
        COutByteStreamVec   以std::vector<char>为底层buf的字节输出流
        COutByteStreamBuf   以[char *, size_t]为底层buf的字节输出流
    操作符:
        array               输入/输出数组
        raw                 输入/输出数组数据
        range               输入/输出范围
        set_order           设置输入/输出流的字节序
        seek                设置输入/输出流的偏移
        skip                跳过/预留指定字节数据
        offset_value        输入/输出指定位置的数据
        insert              在指定位置插入数据
        protobuf            封装protobuf类的输入输出
    History
        20070926    给CInByteStream加入status_状态, 防止因非法数据引起内存访问越界
        20071228    修正CInByteStream::LeftSize在len < cur_时返回很大size_t的问题;并把2个ensure里的减法改成加法
        20080204    去掉CInByteStream::DumpLeft, 加入ToString, 输出对象内部状态
        20081008    将CInDataStream和COutDataStream更名为CInByteStream和COutByteStream
        20081016    调整类结构，引入CDataStreamBase作为所有数据流基类
                    重写CInByteStream和COutByteStream，加入更多类型支持
                    引入manipulator，并实现多种输入输出方式
        20081106    增加Manip::insert
        20121217    增加COutByteStreamVec, COutByteStreamBuf, 修改COutByteStream为以std::string为底层buf
                    增加protobuf类的输入输出
    Manual:
        请参考"doc/DataStream-manual.txt"

//*/

#include <cassert>
#include <string>
#include <cstring>  //memcpy
#include <vector>
#include "impl/DataStream_impl.h"
#include "Tools.h"   //Tools::SwapByteOrder

NS_SERVER_BEGIN

class CInByteStream : public NS_IMPL::CDataStreamBase
{
    typedef CInByteStream __Myt;
    static const bool DEF_NET_BYTEORDER = true;    //默认使用网络字节序(true)还是本地字节序(false)
    const char *    data_;
    size_t          len_;
    size_t          cur_;
    bool            need_reverse_;  //是否需要改变字节序
public:
    CInByteStream(const char * d, size_t l,bool netByteOrder = DEF_NET_BYTEORDER){   //netByteOrder表示是否按网络字节序
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const unsigned char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const signed char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const std::vector<char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    CInByteStream(const std::vector<unsigned char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    CInByteStream(const std::vector<signed char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    CInByteStream(const std::string & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    void SetSource(const char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        data_ = d;
        len_ = l;
        cur_ = 0;
        need_reverse_ = NeedReverse(netByteOrder);
        ResetStatus();
    }
    void SetSource(const unsigned char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)d,l,netByteOrder);
    }
    void SetSource(const signed char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)d,l,netByteOrder);
    }
    void SetSource(const std::vector<char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(&d[0],d.size(),netByteOrder);
    }
    void SetSource(const std::vector<unsigned char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)&d[0],d.size(),netByteOrder);
    }
    void SetSource(const std::vector<signed char> & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)&d[0],d.size(),netByteOrder);
    }
    void SetSource(const std::string & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d.c_str(),d.size(),netByteOrder);
    }
    //设置字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    //按照dir指定的方向设置cur_指针偏移
    //返回cur_最后的绝对偏移
    size_t Seek(ssize_t off,ESeekDir dir = Begin){
        switch(dir){
            case Begin:
                cur_ = off;
                break;
            case End:
                assert(size_t(off) <= len_);
                cur_ = len_ - off;
                break;
            case Cur:
                cur_ += off;
                break;
            default:
                assert(0);
        }
        return cur_;
    }
    //返回当前cur_指针的偏移，dir表示相对位置
    size_t Tell(ESeekDir dir = Begin) const{
        switch(dir){
            case Begin:
                return cur_;
            case End:
                return (len_ > cur_ ? len_ - cur_ : 0);
            default:;
        }
        return 0;
    }
    size_t CurPos() const{return Tell();}
    //剩余的字节数
    size_t LeftSize() const{return Tell(End);}
    //read PODs
    __Myt & operator >>(char & c)               {return readPod(c);}
    __Myt & operator >>(signed char & c)        {return readPod(c);}
    __Myt & operator >>(unsigned char & c)      {return readPod(c);}
    __Myt & operator >>(short & c)              {return readPod(c);}
    __Myt & operator >>(unsigned short & c)     {return readPod(c);}
    __Myt & operator >>(int & c)                {return readPod(c);}
    __Myt & operator >>(unsigned int & c)       {return readPod(c);}
    __Myt & operator >>(long & c)               {return readPod(c);}
    __Myt & operator >>(unsigned long & c)      {return readPod(c);}
    __Myt & operator >>(long long & c)          {return readPod(c);}
    __Myt & operator >>(unsigned long long & c) {return readPod(c);}
    //read std::string
    __Myt & operator >>(std::string & c){
        __Length sz = 0;
        operator >>(sz);
        if(ensure(sz)){
            c.assign(data_ + cur_ ,data_ + cur_ + sz);
            cur_ += sz;
        }
        return *this;
    }
    //read array( = length + raw array)
    template<class T>
    __Myt & operator >>(T * c){
        __Length sz;
        operator >>(sz);
        return readRaw(c,sz);
    }
    //read array( = length + raw array) through CManipulatorArray
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorArray<T> & m){
        __Length sz;
        operator >>(sz);
        if(sz > m.Size1()){
            Status(1);
            return *this;
        }
        m.Size2(sz);
        return readRaw(m.Ptr(),sz);
    }
    //read raw array through CManipulatorRaw
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorRaw<T> & m){
        return readRaw(m.Ptr(),m.Size());
    }
    //read range of raw array through CManipulatorRange
    template<class Iter>
    __Myt & operator >>(const NS_IMPL::CManipulatorRange<Iter> & m){
        for(Iter i = m.Begin();i != m.End();++i)
            if(!(*this>>(*i)))
                break;
        return *this;
    }
    //set order type(NetOrder or HostOrder) through CManipulatorSetOrder
    __Myt & operator >>(const NS_IMPL::CManipulatorSetOrder & m){
        OrderType(m.Order());
        return *this;
    }
    //set cur_ position through CManipulatorSeek
    __Myt & operator >>(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(),m.Dir());
        return *this;
    }
    //read value from a particular position but not change cur_
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = cur_;
        Seek(m.Off(),Begin);
        *this>>(m.Value());
        Seek(old,Begin);
        return *this;
    }
    //read protobuf message
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorProtobuf<T> & m){
        size_t sz = m.Size();
        if(sz && ensure(sz)){
            typename NS_IMPL::CManipulatorProtobuf<T>::__Msg & msg = m.Msg();
            if(msg.ParseFromArray(data_ + cur_, sz)){
                cur_ += sz;
            }else
                Status(1);
        }
        return *this;
    }
private:
    template<typename T>
    __Myt & readPod(T & c){
        if(ensure(sizeof(T))){
            memcpy(&c,data_ + cur_,sizeof(T));
            if(need_reverse_ && sizeof(T) > 1)
                c = Tools::SwapByteOrder(c);
            cur_ += sizeof(T);
        }
        return *this;
    }
    template<typename T>
    __Myt & readRaw(T * c,size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && need_reverse_))
        {
            for(size_t i = 0;i < sz;++i,++c)
                if(!(*this>>(*c)))
                    break;
        }else{
            sz *= sizeof(T);
            if(ensure(sz)){
                memcpy(c,data_ + cur_,sz);
                cur_ += sz;
            }
        }
        return *this;
    }
    template<typename T>
    __Myt & readArray(T * c){
        __Length sz;
        operator >>(sz);
        return readRaw(c,sz);
    }
    bool ensure(size_t sz){     //防止越界访问data_
        if(operator !())
            return false;
        if(len_ < cur_ + sz){
            Status(1);
            return false;
        }
        return true;
    }
};

template<class Buf>
class COutByteStreamBasic : public NS_IMPL::CDataStreamBase
{
    typedef COutByteStreamBasic<Buf>    __Myt;
    typedef NS_IMPL::__buf_adapter<Buf> __BufAdapter;
    typedef typename __BufAdapter::__Char   __Char;
protected:
    typedef Buf __Buf;
    static const bool DEF_NET_BYTEORDER = true;    //默认使用网络字节序(true)还是本地字节序(false)
public:
    explicit COutByteStreamBasic(__Buf & buf, bool netByteOrder)
        : adapter_(buf)
        , need_reverse_(NeedReverse(netByteOrder))
    {}
    //设置/获取字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    EOrderType OrderType() const{return (need_reverse_ ? HostOrder : NetOrder);}
    //返回当前数据字节数
    size_t Size() const{return adapter_.size();}
    //按照dir指定的方向设置Size
    //注意：如果Size变小，相当于抹掉了之后的数据；如果Size变大了，相当于留出指定的空位
    //return:
    //      如果成功，返回最后的Size()
    //      如果失败，返回-1
    ssize_t Seek(ssize_t off, ESeekDir dir){
        switch(dir){
            case Begin:
                if(off < 0)
                    Status(1);
                else if(ensureSize(off))
                    adapter_.resize(off);
                break;
            case End:
            case Cur:
                if(ensureRoom(off))
                    adapter_.resize(Size() + off);
                break;
            default:
                Status(1);
        }
        return (operator !() ? -1 : Size());
    }
    //导出所有写入的数据，追加到dst已有数据后面
    template<class BufType>
    bool ExportData(BufType & dst){
        return adapter_.exportData(dst);
    }
    //导出所有写入的数据，覆盖dst已有数据
    template<typename Char>
    bool ExportData(Char * dst, size_t & sz){
        return adapter_.exportData(dst, sz);
    }
    //write PODs
    __Myt & operator <<(char c)                 {return writePod(c);}
    __Myt & operator <<(signed char c)          {return writePod(c);}
    __Myt & operator <<(unsigned char c)        {return writePod(c);}
    __Myt & operator <<(short c)                {return writePod(c);}
    __Myt & operator <<(unsigned short c)       {return writePod(c);}
    __Myt & operator <<(int c)                  {return writePod(c);}
    __Myt & operator <<(unsigned int c)         {return writePod(c);}
    __Myt & operator <<(long c)                 {return writePod(c);}
    __Myt & operator <<(unsigned long c)        {return writePod(c);}
    __Myt & operator <<(long long c)            {return writePod(c);}
    __Myt & operator <<(unsigned long long c)   {return writePod(c);}
    //write std::string
    __Myt & operator <<(std::string c){
        return writeArray(c.c_str(),c.length());
    }
    //write array( = length + raw array) through CManipulatorArray
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorArray<T> & m){
        return writeArray(m.Ptr(),m.Size1());
    }
    //write raw array through CManipulatorRaw
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorRaw<T> & m){
        return writeRaw(m.Ptr(), m.Size());
    }
    //write range of raw array through CManipulatorRange
    template<class Iter>
    __Myt & operator <<(const NS_IMPL::CManipulatorRange<Iter> & m){
        for(Iter i = m.Begin();i != m.End();++i)
            *this<<(*i);
        return *this;
    }
    //set order type(NetOrder, HostOrder) through CManipulatorSetOrder
    __Myt & operator <<(const NS_IMPL::CManipulatorSetOrder & m){
        OrderType(m.Order());
        return *this;
    }
    //set cur_ position through CManipulatorSeek
    __Myt & operator <<(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(), m.Dir());
        return *this;
    }
    //write value to a particular position but not change cur_
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = Size();
        if(ensureSize(m.Off())){
            adapter_.resize(m.Off());
            *this<<(m.Value());
            adapter_.resize(old);
        }
        return *this;
    }
    //insert value into a particular position and change cur_ relatively
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorInsert<T> & m){
        typedef COutByteStreamBasic<std::string> __OutStreamStr;
        std::string buf;
        __OutStreamStr ds(buf, need_reverse_);
        if(ds<<m.Value())
            if(ensureRoom(ds.Size()))
                adapter_.insert(m.Off(), &buf[0], ds.Size());
        return *this;
    }
    //write protobuf message
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorProtobuf<T> & m){
        typename NS_IMPL::CManipulatorProtobuf<T>::__Msg & msg = m.Msg();
        size_t old = Size();
        size_t sz = msg.ByteSize();
        if(ensureRoom(sz)){
            adapter_.resize(old + sz);
            if(!msg.SerializeToArray(&adapter_[old], sz)){
                adapter_.resize(old);
                Status(1);
            }
        }
        return *this;
    }
protected:
    void init(){adapter_.init();}
private:
    template<typename T>
    __Myt & writePod(T c){
        if(ensureRoom(sizeof(T))){
            if(need_reverse_ && sizeof c > 1)
                c = Tools::SwapByteOrder(c);
            adapter_.append(reinterpret_cast<const __Char *>(&c), sizeof c);
        }
        return *this;
    }
    template<typename T>
    __Myt & writeRaw(const T * c, size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && need_reverse_))
        {
            for(size_t i = 0;i < sz;++i,++c)
                if(!(*this<<(*c)))
                    break;
        }else{
            sz *= sizeof(T);
            if(ensureRoom(sz))
                adapter_.append(reinterpret_cast<const __Char *>(c), sz);
        }
        return *this;
    }
    template<typename T>
    __Myt & writeArray(const T * c, size_t sz){
        if(operator <<(__Length(sz))){
            if(sz){
                assert(c);
                return writeRaw(c, sz);
            }
        }
        return *this;
    }
    bool ensureSize(size_t sz){
        if(operator !())
            return false;
        if(!adapter_.ensureSize(sz)){
            Status(1);
            return false;
        }
        return true;
    }
    bool ensureRoom(ssize_t len){
        if(operator !())
            return false;
        if(!adapter_.ensureRoom(len)){
            Status(1);
            return false;
        }
        return true;
    }
    __BufAdapter adapter_;
    bool need_reverse_;  //是否需要改变结果的byte order
};

class COutByteStream : public COutByteStreamBasic<std::string>
{
    typedef COutByteStreamBasic<std::string> __MyBase;
    typedef COutByteStream __Myt;
public:
    typedef __MyBase::__Buf __Buf;
    explicit COutByteStream(size_t reserve = 100, bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(buf_, netByteOrder)
    {
        buf_.reserve(reserve);
        init();
    }
private:
    __Buf buf_;
};

class COutByteStreamVec : public COutByteStreamBasic<std::vector<char> >
{
    typedef COutByteStreamBasic<std::vector<char> > __MyBase;
    typedef COutByteStream __Myt;
public:
    typedef __MyBase::__Buf __Buf;
    explicit COutByteStreamVec(size_t reserve = 100, bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(buf_, netByteOrder)
    {
        buf_.reserve(reserve);
        init();
    }
private:
    __Buf buf_;
};

class COutByteStreamBuf : public COutByteStreamBasic<NS_IMPL::__byte_buf_wrap<char> >
{
    typedef NS_IMPL::__byte_buf_wrap<char> __Buf;
    typedef COutByteStream __Myt;
    typedef COutByteStreamBasic<__Buf> __MyBase;
public:
    COutByteStreamBuf(char * buf, size_t capacity, bool netByteOrder = DEF_NET_BYTEORDER, size_t size = 0)
        : __MyBase(buf_, netByteOrder)
        , buf_(buf, capacity, size)
    {
        init();
    }
private:
    __Buf buf_;
};

//manipulators' functions:
namespace Manip{
    //read/write array( = length + raw array)
    template<class T>
    inline NS_IMPL::CManipulatorArray<T> array(T * c,size_t sz,size_t * real_sz = 0){
        return NS_IMPL::CManipulatorArray<T>(c,sz,real_sz);
    }

    //read/write raw array
    template<class T>
    inline NS_IMPL::CManipulatorRaw<T> raw(T * c,size_t sz){
        return NS_IMPL::CManipulatorRaw<T>(c,sz);
    }

    //read/write range [first,last) of raw array
    template<class Iter>
    inline NS_IMPL::CManipulatorRange<Iter> range(Iter first,Iter last){
        return NS_IMPL::CManipulatorRange<Iter>(first,last);
    }

    //set byte order type(NetOrder or HostOrder)
    inline NS_IMPL::CManipulatorSetOrder set_order(NS_IMPL::CDataStreamBase::EOrderType order){
        return NS_IMPL::CManipulatorSetOrder(order);
    }

    inline NS_IMPL::CManipulatorSetOrder set_order(bool netByteOrder){
        return NS_IMPL::CManipulatorSetOrder(
            netByteOrder ? NS_IMPL::CDataStreamBase::NetOrder : NS_IMPL::CDataStreamBase::HostOrder);
    }

    //set read/write position
    inline NS_IMPL::CManipulatorSeek seek(ssize_t off,NS_IMPL::CDataStreamBase::ESeekDir dir = NS_IMPL::CDataStreamBase::Begin){
        return NS_IMPL::CManipulatorSeek(off,dir);
    }

    //skip/reserve certain bytes
    inline NS_IMPL::CManipulatorSeek skip(ssize_t off){
        return NS_IMPL::CManipulatorSeek(off,NS_IMPL::CDataStreamBase::Cur);
    }

    //read/write value from offset position
    template<class T>
    inline NS_IMPL::CManipulatorOffsetValue<T> offset_value(size_t offset,T & val){
        return NS_IMPL::CManipulatorOffsetValue<T>(offset,val);
    }
    template<class T>
    inline NS_IMPL::CManipulatorOffsetValue<const T> offset_value(size_t offset,const T & val){
        return NS_IMPL::CManipulatorOffsetValue<const T>(offset,val);
    }

    //insert value into offset position
    template<class T>
    inline NS_IMPL::CManipulatorInsert<T> insert(size_t offset,const T & val){
        return NS_IMPL::CManipulatorInsert<T>(offset,val);
    }

    //read/write protobuf message
    template<class T>
    inline NS_IMPL::CManipulatorProtobuf<T> protobuf(T & msg, size_t size = 0){
        return NS_IMPL::CManipulatorProtobuf<T>(msg, size);
    }
    template<class T>
    inline NS_IMPL::CManipulatorProtobuf<const T> protobuf(const T & msg, size_t size = 0){
        return NS_IMPL::CManipulatorProtobuf<const T>(msg, size);
    }

}//namespace Manip

NS_SERVER_END

#endif

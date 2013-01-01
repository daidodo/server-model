#ifndef DOZERG_DATA_STREAM_H_20070905
#define DOZERG_DATA_STREAM_H_20070905

/*
    封装数据流的读取和写入
    注意测试operator !(), 在错误状态下, 所有读写数据操作都会无效
    类型:
        CInByteStream           以字节为单位的输入流
        COutByteStreamBasic     以任意buf为底层的字节输出流
        COutByteStream          以std::string为底层buf的字节输出流
        COutByteStreamStr       同COutByteStream
        COutByteStreamStrRef    以外部std::string对象为底层buf的字节输出流
        COutByteStreamVec       以std::vector<char>为底层buf的字节输出流
        COutByteStreamVecRef    以外部std::vector<char>对象为底层buf的字节输出流
        COutByteStreamBuf       以(char *, size_t)为底层buf的字节输出流
    操作符:
        array               输入/输出长度+数组数据
        raw                 输入/输出数组数据
        set_order           设置输入/输出流的字节序
        value_byteorder     输入/输出指定字节序的数据
        seek                设置输入/输出流的偏移
        skip                预留/抹除指定字节数据
        offset_value        输入/输出数据到指定位置
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
        20121223    增加COutByteStreamStr, COutByteStreamStrRef, COutByteStreamVecRef, 优化COutByteStreamBasic实现
        201212225   优化array，raw，增加value_byteorder，去掉range（改为raw）
                    优化std::string，std::vector容器的读写
                    重新增加ToString
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

//manipulators' functions:
namespace Manip{

    //read/write raw array
    template<class T>
    inline NS_IMPL::CManipulatorRawPtr<T> raw(T * c, size_t sz){
        return NS_IMPL::CManipulatorRawPtr<T>(c, sz);
    }

    template<class T>
    inline NS_IMPL::CManipulatorRawPtr<T> raw(std::vector<T> & c, size_t sz){
        size_t old = c.size();
        c.resize(old + sz);
        return NS_IMPL::CManipulatorRawPtr<T>(&c[old], sz);
    }

    //sz: return size of c
    template<class T>
    inline NS_IMPL::CManipulatorRawPtr<const T> raw(const std::vector<T> & c, size_t * sz = NULL){
        if(sz)
            *sz = c.size();
        return NS_IMPL::CManipulatorRawPtr<const T>(&c[0], c.size());
    }

    template<typename Char>
    inline NS_IMPL::CManipulatorRawPtr<Char> raw(std::basic_string<Char> & c, size_t len){
        size_t old = c.size();
        c.append(len, 0);
        return NS_IMPL::CManipulatorRawPtr<Char>(&c[old], len);
    }

    //sz: return size of c
    template<typename Char>
    inline NS_IMPL::CManipulatorRawPtr<const Char> raw(const std::basic_string<Char> & c, size_t * sz = NULL){
        if(sz)
            *sz = c.size();
        return NS_IMPL::CManipulatorRawPtr<const Char>(&c[0], c.size());
    }

    //sz: return size of [first, last)
    template<class Iter>
    inline NS_IMPL::CManipulatorRawRange<Iter> raw(Iter first, Iter last, size_t * sz = NULL){
        return NS_IMPL::CManipulatorRawRange<Iter>(first, last, sz);
    }

    template<class T>
    inline NS_IMPL::CManipulatorRawSeqCont<T> raw(T & c, size_t sz){
        return NS_IMPL::CManipulatorRawSeqCont<T>(c, sz, NULL);
    }

    //sz: return size of c
    template<class T>
    inline NS_IMPL::CManipulatorRawSeqCont<const T> raw(const T & c, size_t * sz = NULL){
        return NS_IMPL::CManipulatorRawSeqCont<const T>(c, 0, sz);
    }

    //read/write array( = length + raw array)
    template<typename LenT, class T>
    inline NS_IMPL::CManipulatorArrayPtr<LenT, T> array(T * c, LenT sz, LenT * real_sz = NULL){
        return NS_IMPL::CManipulatorArrayPtr<LenT, T>(c, sz, real_sz);
    }

    template<typename LenT, class T>
    inline NS_IMPL::CManipulatorArrayCont<LenT, T> array(T & c, LenT max_size = 0){
        return NS_IMPL::CManipulatorArrayCont<LenT, T>(c, max_size);
    }

    template<typename LenT, class T>
    inline NS_IMPL::CManipulatorArrayCont<LenT, const T> array(const T & c){
        return NS_IMPL::CManipulatorArrayCont<LenT, const T>(c, 0);
    }

    template<class T>
    inline NS_IMPL::CManipulatorArrayCont<uint16_t, T> array(T & c, uint16_t max_size = 0){
        return NS_IMPL::CManipulatorArrayCont<uint16_t, T>(c, max_size);
    }

    template<class T>
    inline NS_IMPL::CManipulatorArrayCont<uint16_t, const T> array(const T & c){
        return NS_IMPL::CManipulatorArrayCont<uint16_t, const T>(c, 0);
    }

    //set byte order type(true for Net Byte Order, false for Host Byte Order)
    inline NS_IMPL::CManipulatorSetOrder set_order(bool netByteOrder){
        return NS_IMPL::CManipulatorSetOrder(netByteOrder);
    }

    //read/write value with fixed byte order
    template<class T>
    inline NS_IMPL::CManipulatorValueByteOrder<T> value_byteorder(T & val, bool netByteOrder){
        return NS_IMPL::CManipulatorValueByteOrder<T>(val, netByteOrder);
    }

    template<class T>
    inline NS_IMPL::CManipulatorValueByteOrder<const T> value_byteorder(const T & val, bool netByteOrder){
        return NS_IMPL::CManipulatorValueByteOrder<const T>(val, netByteOrder);
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

    //insert value to offset position
    template<class T>
    inline NS_IMPL::CManipulatorInsert<T> insert(size_t offset,const T & val){
        return NS_IMPL::CManipulatorInsert<T>(offset,val);
    }

    //read/write protobuf message
    template<class T>
    inline NS_IMPL::CManipulatorProtobuf<T> protobuf(T & msg, size_t size = size_t(-1)){
        return NS_IMPL::CManipulatorProtobuf<T>(msg, size);
    }
    template<class T>
    inline NS_IMPL::CManipulatorProtobuf<const T> protobuf(const T & msg){
        return NS_IMPL::CManipulatorProtobuf<const T>(msg, 0);
    }

}//namespace Manip

class CInByteStream : public NS_IMPL::CDataStreamBase
{
    typedef NS_IMPL::CDataStreamBase __MyBase;
    typedef CInByteStream __Myt;
    const char *    data_;
    size_t          len_;
    size_t          cur_;
    bool            toByteOrder_;
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
        SetByteOrder(netByteOrder);
        toByteOrder_ = false;
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
    //read raw array through CManipulatorRawPtr
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorRawPtr<T> & m){
        return readRaw(m.Ptr(), m.Size());
    }
    //read raw array through CManipulatorRawSeqCont
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorRawSeqCont<T> & m){
        typedef typename T::value_type __Val;
        for(size_t i = 0;i < m.Size();++i){
            m.Cont().push_back(__Val());
            if(!(*this>>m.Cont().back()))
                break;
        }
        return *this;
    }
    //read range of raw array through CManipulatorRawRange
    template<class Iter>
    __Myt & operator >>(const NS_IMPL::CManipulatorRawRange<Iter> & m){
        size_t sz = 0;
        for(Iter i = m.Begin();i != m.End();++i, ++sz)
            if(!(*this>>(*i)))
                break;
        m.Size(sz);
        return *this;
    }
    //read array( = length + raw array) through CManipulatorArrayPtr
    template<typename LenT, class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorArrayPtr<LenT, T> & m){
        LenT sz;
        if(*this>>sz){
            if(sz > m.Size1()){
                Status(1);
                return *this;
            }
            if(*this>>Manip::raw(m.Ptr(), sz))
                m.Size2(sz);
        }
        return *this;
    }
    //read array( = length + raw array) through CManipulatorArrayCont
    template<typename LenT, class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorArrayCont<LenT, T> & m){
        LenT sz;
        if(*this>>sz){
            if(m.Max() && sz > m.Max()){
                Status(1);
                return *this;
            }
            *this>>Manip::raw(m.Cont(), sz);
        }
        return *this;
    }
    //read std::string( = length + bytes)
    __Myt & operator >>(std::string & c){
        return (*this>>Manip::array(c));
    }
    //set order type(NetOrder or HostOrder) through CManipulatorSetOrder
    __Myt & operator >>(const NS_IMPL::CManipulatorSetOrder & m){
        SetByteOrder(m.GetByteOrder());
        return *this;
    }
    //read value with fixed byte order
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorValueByteOrder<T> & m){
        bool oldBO = toByteOrder_;
        toByteOrder_ = m.GetByteOrder();
        *this>>m.Value();
        toByteOrder_ = oldBO;
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
        if(size_t(-1) == sz)
            sz = LeftSize();
        if(sz && ensure(sz)){
            typename NS_IMPL::CManipulatorProtobuf<T>::__Msg & msg = m.Msg();
            if(msg.ParseFromArray(data_ + cur_, sz)){
                cur_ += sz;
            }else
                Status(1);
        }
        return *this;
    }
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{CDataStreamBase="<<__MyBase::ToString()
            <<", toByteOrder_="<<toByteOrder_
            <<", cur_="<<cur_
            <<", data_=("<<len_<<")"
            <<Tools::DumpHex(data_, cur_, ' ', false)
            <<" | "
            <<Tools::DumpHex(data_ +  cur_, len_ - cur_, ' ', false)
            <<"}";
        return oss.str();
    }
private:
    bool needReverse() const{
        return NeedReverse(GetByteOrder(), toByteOrder_);
    }
    template<typename T>
    __Myt & readPod(T & c){
        if(ensure(sizeof(T))){
            memcpy(&c,data_ + cur_,sizeof(T));
            if(sizeof(T) > 1 && needReverse())
                c = Tools::SwapByteOrder(c);
            cur_ += sizeof(T);
        }
        return *this;
    }
    template<typename T>
    __Myt & readRaw(T * c, size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && needReverse()))
        {
            for(size_t i = 0;i < sz;++i, ++c)
                if(!(*this>>(*c)))
                    break;
        }else{
            sz *= sizeof(T);
            if(ensure(sz)){
                memcpy(c, data_ + cur_, sz);
                cur_ += sz;
            }
        }
        return *this;
    }
    template<typename LenT, class T>
    __Myt & readArray(T * c){
        LenT sz;
        operator >>(sz);
        return readRaw(c, sz);
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

template<class Data>
class COutByteStreamBasic : public NS_IMPL::CDataStreamBase
{
    typedef NS_IMPL::CDataStreamBase    __MyBase;
    typedef COutByteStreamBasic<Data>   __Myt;
    typedef Data                        __Data;
public:
    typedef typename Data::__Buf        __Buf;
    typedef typename Data::__Char       __Char;
    explicit COutByteStreamBasic(size_t reserve = 1024, bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(netByteOrder)
        , data_(reserve)
        , fromByteOrder_(false)
    {}
    COutByteStreamBasic(__Char * buf, size_t sz, bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(netByteOrder)
        , data_(buf, sz)
        , fromByteOrder_(false)
    {}
    explicit COutByteStreamBasic(__Buf & buf, bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(netByteOrder)
        , data_(buf)
        , fromByteOrder_(false)
    {}
    //返回当前数据字节数
    size_t Size() const{return data_.cur();}
    //按照dir指定的方向设置Size
    //注意：如果Size变小，相当于抹掉了之后的数据；如果Size变大了，相当于留出指定的空位
    //return:
    //      如果成功，返回最后的Size()
    //      如果失败，返回-1
    ssize_t Seek(ssize_t off, ESeekDir dir = Begin){
        if(operator !())
            return -1;
        switch(dir){
            case Begin:
                break;
            case End:
            case Cur:
                off += Size();
                break;
            default:
                Status(1);
                return -1;
        }
        if(data_.seek(off))
            return Size();
        Status(1);
        return -1;
    }
    //导出所有写入的数据
    //并清空自己
    bool ExportData(){
        return data_.exportData();
    }
    bool ExportData(size_t & sz){
        return data_.exportData(sz);
    }
    //导出所有写入的数据，追加到dst已有数据后面
    //并清空自己
    bool ExportData(__Buf & dst){
        return data_.exportData(dst);
    }
    template<class BufT>
    bool ExportData(BufT & dst){
        return data_.exportData(dst);
    }
    //导出所有写入的数据，覆盖dst已有数据
    //并清空自己
    template<typename CharT>
    bool ExportData(CharT * dst, size_t & sz){
        return data_.exportData(dst, sz);
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
    //write raw array through CManipulatorRawPtr
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorRawPtr<T> & m){
        return writeRaw(m.Ptr(), m.Size());
    }
    //write raw array through CManipulatorRawSeqCont
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorRawSeqCont<T> & m){
        size_t sz = writeRange(m.Cont().begin(), m.Cont().end());
        m.Size2(sz);
        return *this;
    }
    //write raw array through CManipulatorRawRange
    template<class Iter>
    __Myt & operator <<(const NS_IMPL::CManipulatorRawRange<Iter> & m){
        size_t sz = writeRange(m.Begin(), m.End());
        m.Size(sz);
        return *this;
    }
    //write array( = length + raw array) through CManipulatorArrayPtr
    template<typename LenT, class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorArrayPtr<LenT, T> & m){
        if(*this<<LenT(m.Size1()))
            writeRaw(m.Ptr(), m.Size1());
        return *this;
    }
    //write array( = length + raw array) through CManipulatorArrayCont
    template<typename LenT, class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorArrayCont<LenT, T> & m){
        if(m.Cont().empty()){
            *this<<LenT(0);
        }else{
            const size_t off = Size();
            size_t sz = 0;
            if(*this<<LenT(0)   //NOTE: m.Cont().size() may be O(n) complexity
                    <<Manip::raw(m.Cont(), &sz))
                *this<<Manip::offset_value(off, LenT(sz));
        }
        return *this;
    }
    //write std::string
    __Myt & operator <<(const std::string & c){
        return (*this<<Manip::array(c));
    }
    //set order type(NetOrder, HostOrder) through CManipulatorSetOrder
    __Myt & operator <<(const NS_IMPL::CManipulatorSetOrder & m){
        SetByteOrder(m.GetByteOrder());
        return *this;
    }
    //write value with fixed byte order
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorValueByteOrder<T> & m){
        bool oldBO = fromByteOrder_;
        fromByteOrder_ = m.GetByteOrder();
        *this<<m.Value();
        fromByteOrder_ = oldBO;
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
        const size_t old = Size();
        if(0 <= Seek(m.Off())
                && (*this<<m.Value()))
            Seek(old);
        return *this;
    }
    //insert value into a particular position and change cur_ relatively
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorInsert<T> & m){
        typedef COutByteStreamBasic<NS_IMPL::__buf_ref_data<std::string> > __OutStream;
        if(m.Off() < 0 || m.Off() > data_.cur()){
            Status(1);
        }else{
            std::string buf;
            __OutStream ds(buf);
            if(ds<<m.Value()){
                ds.ExportData();
                if(!buf.empty() && ensureRoom(buf.size()))
                    data_.insert(m.Off(), reinterpret_cast<const __Char *>(&buf[0]), buf.size());
            }
        }
        return *this;
    }
    //write protobuf message
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorProtobuf<T> & m){
        typename NS_IMPL::CManipulatorProtobuf<T>::__Msg & msg = m.Msg();
        const size_t old = Size();
        const size_t sz = msg.ByteSize();
        if(0 <= Seek(old + sz)
                && !msg.SerializeToArray(data_.buf(old), sz)){
            Seek(old);
            Status(1);
        }
        return *this;
    }
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{CDataStreamBase="<<__MyBase::ToString()
            <<", fromByteOrder_="<<fromByteOrder_
            <<", data_="<<data_.ToString()
            <<"}";
        return oss.str();
    }
private:
    bool needReverse() const{
        return NeedReverse(fromByteOrder_, GetByteOrder());
    }
    template<typename T>
    __Myt & writePod(T c){
        if(ensureRoom(sizeof(T))){
            if(sizeof c > 1 && needReverse())
                c = Tools::SwapByteOrder(c);
            data_.append(reinterpret_cast<const __Char *>(&c), sizeof c);
        }
        return *this;
    }
    template<typename T>
    __Myt & writeRaw(const T * c, size_t sz){
        if(sz){
            assert(c);
            if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
                    || (sizeof(T) > 1 && needReverse()))
            {
                for(size_t i = 0;i < sz;++i,++c)
                    if(!(*this<<(*c)))
                        break;
            }else{
                sz *= sizeof(T);
                if(ensureRoom(sz))
                    data_.append(reinterpret_cast<const __Char *>(c), sz);
            }
        }
        return *this;
    }
    template<class Iter>
    size_t writeRange(Iter first, Iter last){
        size_t c = 0;
        for(Iter i = first;i != last;++i, ++c)
            if(!(*this<<(*i)))
                break;
        return c;
    }
    bool ensureRoom(size_t len){
        if(operator !())
            return false;
        if(!data_.ensure(len)){
            Status(1);
            return false;
        }
        return true;
    }
    //members
    __Data data_;
    bool fromByteOrder_;
};

//COutByteStream, COutByteStreamStr
typedef COutByteStreamBasic<NS_IMPL::__buf_data<std::string> > COutByteStreamStr;

typedef COutByteStreamStr COutByteStream;

//COutByteStreamStr
typedef COutByteStreamBasic<NS_IMPL::__buf_ref_data<std::string> > COutByteStreamStrRef;

//COutByteStreamVec
typedef COutByteStreamBasic<NS_IMPL::__buf_data<std::vector<char> > > COutByteStreamVec;

//COutByteStreamVecRef
typedef COutByteStreamBasic<NS_IMPL::__buf_ref_data<std::vector<char> > > COutByteStreamVecRef;

//COutByteStreamBuf
typedef COutByteStreamBasic<NS_IMPL::__buf_data<CCharBuffer<char> > > COutByteStreamBuf;

NS_SERVER_END

#endif

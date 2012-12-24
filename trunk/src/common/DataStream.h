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
        range               输入/输出[first, last)范围内的数据
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
        20121223    增加COutByteStreamStr, COutByteStreamStrRef, COutByteStreamVecRef, 优化COutByteStreamBasic实现
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
        SetByteOrder(m.NetByteOrder());
        return *this;
    }
    //read value with fixed byte order
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorValueByteOrder<T> & m){
        bool oldBO = toByteOrder_;
        toByteOrder_ = m.NetByteOrder();
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
    bool needReverse() const{
        return NeedReverse(NetByteOrder(), toByteOrder_);
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
    __Myt & readRaw(T * c,size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && needReverse()))
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
        SetByteOrder(m.NetByteOrder());
        return *this;
    }
    //read value with fixed byte order
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorValueByteOrder<T> & m){
        bool oldBO = fromByteOrder_;
        fromByteOrder_ = m.NetByteOrder();
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
                && (*this<<(m.Value())))
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
private:
    bool needReverse() const{
        return NeedReverse(fromByteOrder_, NetByteOrder());
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
    template<typename T>
    __Myt & writeArray(const T * c, size_t sz){
        if(operator <<(__Length(sz)))
            writeRaw(c, sz);
        return *this;
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

//manipulators' functions:
namespace Manip{

    //read/write array( = length + raw array)
    template<class T>
    inline NS_IMPL::CManipulatorArray<T> array(T * c,size_t sz,size_t * real_sz = 0){
        return NS_IMPL::CManipulatorArray<T>(c,sz,real_sz);
    }

    //template<class T>
    //inline NS_IMPL::CManipulatorContainer<T>

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

    //set byte order type(true for NetByteOrder, false for HostByteOrder)
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

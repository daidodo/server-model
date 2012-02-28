#ifndef DOZERG_REFERENCE_COUNT_POINTER_H_20070828
#define DOZERG_REFERENCE_COUNT_POINTER_H_20070828

/*
        CSharedPtr  �������ü���������ָ��CSharedPtr
                    ��Ҫ��allocatorһ��ʹ��
                    �ȿ��Է�ֹ�ڴ�й©,�ּ����ڴ���Ƭ
                    Ҫ����ָ��Ķ�������ʵ��allocator_type
                    �Ա���ȷ�����ٶ���
        CAnyPtr     ���Դ���κ�ָ������ͣ�����ָ֤��ת������ȷ��
    History
        20070924    ��_ref_imp�м���pthread_mutex_t,�������߳��²����Ĵ���
        20070925    ��cnt_��pthread_mutex_t�ĳ�CLockInt
        20080123    _ref_imp��cnt_��Ա���͸�Ϊģ�����IntType.CSharedPtr����Lockģ�����,��ʾ�Ƿ���Ҫ����
        20080604    ����release��set��Ա������safe_bool_type����4���Ƚϲ���
        20080912    ����swap������������std::swap
        20080920    ʹ��ģ���������������
        20120118    ����release()
//*/

#include <algorithm>     //std::swap
#include <typeinfo>      //std::type_info
#include <sstream>
#include <LockInt.h>
#include <impl/SharedPtr_impl.h>

NS_SERVER_BEGIN

template<class T,bool bLock = true,class LockT = CMutex>
class CSharedPtr
{   //typedefs
    typedef CSharedPtr<T,bLock,LockT>           __Myt;
    typedef CLockInt<int,LockT>                 __LockInt;
    typedef typename CTypeSelector
        <__LockInt,int,bLock>::RType            __Int;
    typedef NS_IMPL::_ref_imp<T,__Int>          __ref_type;
    typedef typename __ref_type::__ref_alloc    __ref_alloc;
public:
    typedef T   value_type;
    typedef T & reference;
    typedef T * pointer;
    typedef typename __ref_type::__elem_alloc   allocator_type;
private:
    typedef void (__Myt::*safe_bool_type)(pointer);
public:
    //functions
    CSharedPtr(pointer p = 0):ref_(0){init(p);}
    CSharedPtr(const __Myt & a):ref_(0){__ref_type::changeRef(ref_,a.ref_);}
    ~CSharedPtr(){__ref_type::subRef(ref_);}
    __Myt & operator =(const __Myt & a){
        __ref_type::changeRef(ref_,a.ref_);
        return *this;
    }
    __Myt & operator =(pointer a){
        if(operator !=(a)){
            __ref_type::subRef(ref_);
            init(a);
        }
        return *this;
    }
    reference operator *() const{return *ref_->ptr_;}
    pointer operator ->() const{return ref_->ptr_;}
    void release(){__ref_type::subRef(ref_, true);}
    bool operator !() const{return !ref_;}
    operator safe_bool_type() const{return operator !() ? 0 : &__Myt::init;}
    bool operator ==(const __Myt & a) const{return ref_ == a.ref_;}
    bool operator !=(const __Myt & a) const{return !operator ==(a);}
    bool operator ==(pointer a) const{
        return operator !() ? !a : a == operator ->();
    }
    bool operator !=(pointer a) const{return !operator ==(a);}
    void swap(__Myt & a) __DZ_NOTHROW{std::swap(ref_,a.ref_);}
private:
    void init(pointer p){
        if(p)
            ref_ = __ref_type::GetObject(p);
    }
    //field
    __ref_type * ref_;
};

template<class T,bool B,class L>
inline void swap(NS_SERVER::CSharedPtr<T,B,L> & a,NS_SERVER::CSharedPtr<T,B,L> & b) __DZ_NOTHROW
{
    a.swap(b);
}

class CAnyPtr
{
    typedef void (CAnyPtr::*safe_bool_type)() const;
public:
    CAnyPtr():p_(0),t_(0){}
    template<class T>
    explicit CAnyPtr(T * p)
        : p_(p)
        , t_(&typeid(T))
    {}
    template<class T>
    CAnyPtr & operator =(T * p){
        p_ = p;
        t_ = &typeid(T);
        return *this;
    }
    template<class T>
    T * CastTo() const{
        if(!t_ || *t_ != typeid(T))
            return 0;
        return reinterpret_cast<T *>(p_);
    }
    void Reset(){
        p_ = 0;
        t_ = 0;
    }
    operator safe_bool_type() const{return (p_ ? &CAnyPtr::dummyFun : 0);}
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{p_=@"<<p_
            <<", t_=@"<<t_;
        if(t_)
            oss<<"("<<Tools::CxxDemangle(t_->name())<<")";
        oss<<"}";
        return oss.str();
    }
private:
    void dummyFun() const{}
    void * p_;
    const std::type_info * t_;
};

template<class T>
inline T * PtrCast(const CAnyPtr & p)
{
    return p.CastTo<T>();
}

NS_SERVER_END

#endif

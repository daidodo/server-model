#ifndef DOZERG_REFERENCE_COUNT_POINTER_H_20070828
#define DOZERG_REFERENCE_COUNT_POINTER_H_20070828

/*
    �������ü���������ָ��CSharedPtr
    ��Ҫ��allocatorһ��ʹ��
    �ȿ��Է�ֹ�ڴ�й©,�ּ����ڴ���Ƭ
    Ҫ����ָ��Ķ�������ʵ��allocator_type
    �Ա���ȷ�����ٶ���
        CSharedPtr
    History
        20070924    ��_ref_imp�м���pthread_mutex_t,�������߳��²����Ĵ���
        20070925    ��cnt_��pthread_mutex_t�ĳ�CLockInt
        20080123    _ref_imp��cnt_��Ա���͸�Ϊģ�����IntType.CSharedPtr����Lockģ�����,��ʾ�Ƿ���Ҫ����
        20080604    ����release��set��Ա������safe_bool_type����4���Ƚϲ���
        20080912    ����swap������������std::swap
        20080920    ʹ��ģ���������������
//*/

#include <algorithm>        //std::swap
#include <common/LockInt.h> //CLockInt,CMutex
#include <common/impl/SharedPtr_impl.h>

NS_SERVER_BEGIN

template<class T,bool bLock = true,class LockT = CMutex>
class CSharedPtr
{   //typedefs
    typedef CSharedPtr<T,bLock,LockT>           __Myt;
    typedef CLockInt<int,LockT>                 __LockInt;
    typedef typename Tools::CTypeSelector
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

NS_SERVER_END

namespace std{
    template<class T,bool B,class L>
    inline void swap(NS_SERVER::CSharedPtr<T,B,L> & a,NS_SERVER::CSharedPtr<T,B,L> & b) __DZ_NOTHROW
    {
        a.swap(b);
    }
}//namespace std

#endif

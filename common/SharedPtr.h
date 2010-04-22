#ifndef DOZERG_REFERENCE_COUNT_POINTER_H_20070828
#define DOZERG_REFERENCE_COUNT_POINTER_H_20070828

/*
    采用引用计数的智能指针CSharedPtr
    需要与allocator一起使用
    既可以防止内存泄漏,又减少内存碎片
    要求所指向的对象类型实现allocator_type
    以便正确的销毁对象
        CSharedPtr
    History
        20070924    在_ref_imp中加入pthread_mutex_t,修正多线程下操作的错误
        20070925    把cnt_和pthread_mutex_t改成CLockInt
        20080123    _ref_imp的cnt_成员类型改为模板参数IntType.CSharedPtr加入Lock模板参数,表示是否需要加锁
        20080604    增加release，set成员函数，safe_bool_type，和4个比较操作
        20080912    增加swap函数，并重载std::swap
        20080920    使用模板参数决定锁类型
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

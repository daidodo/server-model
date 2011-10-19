#ifndef DOZERG_LIST_IMPL_H_20080226
#define DOZERG_LIST_IMPL_H_20080226

/*
    CList的内部实现
        __list_node
        __list_const_iterator_impl
        __list_iterator_impl
//*/

#include <iterator>     //std::bidirectional_iterator_tag
#include <cstddef>      //std::ptrdiff_t
#include <common/impl/Config.h>

//forward declaration
NS_SERVER_BEGIN
template<class T,class Alloc>class CList;
NS_SERVER_END

NS_IMPL_BEGIN

//__list_node
template<class T>
struct __list_node{
    __list_node *prev,*next;
    union{
        T *     data_ptr;
        size_t  size_;
    };
};

//__list_const_iterator_impl
template<class T>
class __list_const_iterator_impl
{
    typedef __list_const_iterator_impl<T>   __Myt;
protected:
    typedef __list_node<T>  __NodeType;
    typedef __NodeType *    __NodePtr;
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T               value_type;
    typedef const T *       pointer;
    typedef const T &       reference;
    typedef std::ptrdiff_t  difference_type;
    __list_const_iterator_impl(){}
    __list_const_iterator_impl(__NodePtr n):node_(n){}
    reference operator *() const    {return *node_->data_ptr;}
    pointer operator ->() const     {return &operator *();}
    bool operator ==(const __Myt & other) const {return node_ == other.node_;}
    bool operator !=(const __Myt & other) const {return !operator ==(other);}
    __Myt & operator ++(){
        node_ = node_->next;
        return *this;
    }
    __Myt operator ++(int){
        __list_const_iterator_impl tmp = *this;
        ++*this;
        return tmp;
    }
    __Myt & operator --(){
        node_ = node_->prev;
        return *this;
    }
    __Myt operator --(int){
        __list_const_iterator_impl tmp = *this;
        --*this;
        return tmp;
    }
protected:
    __NodePtr node_;
};

//__list_iterator_impl
template<class T,class Alloc>
class __list_iterator_impl : public __list_const_iterator_impl<T>
{
    friend class CList<T,Alloc>;
    typedef __list_const_iterator_impl<T>   __MyBase;
    typedef __list_iterator_impl<T,Alloc>   __Myt;
    typedef __list_node<T>  __NodeType;
    typedef __NodeType *    __NodePtr;
public:
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef T   value_type;
    typedef T * pointer;
    typedef T & reference;
    typedef std::ptrdiff_t  difference_type;
    __list_iterator_impl(){}
    __list_iterator_impl(__NodePtr n):__MyBase(n){}
    reference operator *() const    {return *__MyBase::node_->data_ptr;}
    pointer operator ->() const     {return &operator *();}
    __list_iterator_impl & operator ++(){
        __MyBase::operator ++();
        return *this;
    }
    __list_iterator_impl operator ++(int){
        __list_iterator_impl tmp(*this);
        ++*this;
        return tmp;
    }
    __list_iterator_impl & operator --(){
        __MyBase::operator --();
        return *this;
    }
    __list_iterator_impl operator --(int){
        __list_iterator_impl tmp(*this);
        --*this;
        return tmp;
    }
};

NS_IMPL_END

#endif

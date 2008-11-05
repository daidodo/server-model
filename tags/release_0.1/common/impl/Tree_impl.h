#ifndef DOZERG_TREE_IMPL_H_20080226
#define DOZERG_TREE_IMPL_H_20080226

/*
    CTree的内部实现
        __rb_tree_node
        __rb_tree_const_iterator_impl
        __rb_tree_iterator_impl
    CBPlusTree的内部实现
//*/

#include <iterator>     //for std::bidirectional_iterator_tag
#include <common/impl/Config.h>

//forward declaration
NS_SERVER_BEGIN
template<class Value,class KeyOfValue,class Compare,class Alloc>
class CTree;
template<class Value,class KeyOfValue,class Compare,class Alloc>
class CBPlusTree;
NS_SERVER_END

NS_IMPL_BEGIN

//class CTree
template<class Value>
struct __rb_tree_node
{
    enum __NodeColor{BLACK,RED};
    __NodeColor color_;
    __rb_tree_node *parent_,*left_,*right_;
    union{
        Value * data_ptr_;
        size_t  size_;
    };
    void leftSon(__rb_tree_node * ls){
        left_ = ls;
        if(ls)
            ls->parent_ = this;
    }
    void rightSon(__rb_tree_node * rs){
        right_ = rs;
        if(rs)
            rs->parent_ = this;
    }
    __rb_tree_node * mostLeft(){
        if(!left_)
            return this;
        __rb_tree_node * ret = left_;
        for(;ret->left_;ret = ret->left_);
        return ret;
    }
    __rb_tree_node * mostRight(){
        if(!right_)
            return this;
        __rb_tree_node * ret = right_;
        for(;ret->right_;ret = ret->right_);
        return ret;
    }
    __rb_tree_node * next() {
        __rb_tree_node * node_ptr_ = this;
        if(node_ptr_->right_ != 0){
            node_ptr_ = node_ptr_->right_;
            while(node_ptr_->left_ != 0)
                node_ptr_ = node_ptr_->left_;
        }else{
            __rb_tree_node * p = node_ptr_->parent_;
            while(node_ptr_ == p->right_){
                node_ptr_ = p;
                p = node_ptr_->parent_;
            }
            if(node_ptr_->right_ != p)
                node_ptr_ = p;
        }
        return node_ptr_;
    }
    __rb_tree_node * prev() {
        __rb_tree_node * node_ptr_ = this;
        if(node_ptr_->parent_->parent_ == node_ptr_ && node_ptr_->color_ == RED)    //this == __RBTree::header_
            node_ptr_ = node_ptr_->right_;
        else if(node_ptr_->left_ != 0){
            node_ptr_ = node_ptr_->left_;
            while(node_ptr_->right_ != 0)
                node_ptr_ = node_ptr_->right_;
        }else{
            __rb_tree_node * p = node_ptr_->parent_;
            while(node_ptr_ == p->left_){
                node_ptr_ = p;
                p = node_ptr_->parent_;
            }
            node_ptr_ = p;
        }
        return node_ptr_;
    }
};

template<class Value,class KeyOfValue,class Compare, class Alloc>
class __rb_tree_const_iterator_impl
{
    typedef __rb_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc>    __Myt;
protected:
    typedef CTree<Value,KeyOfValue,Compare,Alloc>   __RBTree;
public:
    typedef std::bidirectional_iterator_tag         iterator_category;
    typedef typename __RBTree::value_type           value_type;
    typedef typename __RBTree::const_pointer        pointer;
    typedef typename __RBTree::const_reference      reference;
    typedef typename __RBTree::difference_type      difference_type;
protected:
    typedef __rb_tree_node<value_type>  __Node;
    typedef __Node *                    __NodePtr;
    typedef typename Alloc::template rebind<__Node>::other  __NodeAlloc;
public:
    __rb_tree_const_iterator_impl(){}
    __rb_tree_const_iterator_impl(__NodePtr node):node_ptr_(node){}
    reference operator *() const    {return *node_ptr_->data_ptr_;}
    pointer operator ->() const     {return &(operator *());}
    bool operator ==(const __Myt & other) const {return node_ptr_ == other.node_ptr_;}
    bool operator !=(const __Myt & other) const {return !operator ==(other);}
    __Myt & operator ++(){
        node_ptr_ = node_ptr_->next();
        return *this;
    }
    __Myt operator ++(int){
        __Myt tmp(*this);
        ++*this;
        return tmp;
    }
    __Myt & operator --(){
        node_ptr_ = node_ptr_->prev();
        return *this;
    }
    __Myt operator --(int){
        __Myt tmp(*this);
        --*this;
        return tmp;
    }
protected:
    __NodePtr node_ptr_;
};

template<class Value,class KeyOfValue,class Compare, class Alloc>
class __rb_tree_iterator_impl
    : public __rb_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc>
{
    friend class CTree<Value,KeyOfValue,Compare,Alloc>;
    typedef __rb_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc>   __MyBase;
    typedef __rb_tree_iterator_impl<Value,KeyOfValue,Compare,Alloc>         __Myt;
    typedef typename __MyBase::__RBTree     __RBTree;
    typedef typename __MyBase::__NodePtr    __NodePtr;
public:
    typedef std::bidirectional_iterator_tag     iterator_category;
    typedef typename __MyBase::value_type       value_type;
    typedef typename __MyBase::difference_type  difference_type;
    typedef typename __RBTree::pointer          pointer;
    typedef typename __RBTree::reference        reference;
    __rb_tree_iterator_impl(){}
    __rb_tree_iterator_impl(__NodePtr node):__MyBase(node){}
    reference operator *() const    {return *__MyBase::node_ptr_->data_ptr_;}
    pointer operator ->() const     {return &(operator *());}
    __Myt & operator ++(){
        __MyBase::operator ++();
        return *this;
    }
    __Myt operator ++(int){
        __Myt tmp(*this);
        ++*this;
        return tmp;
    }
    __Myt & operator --(){
        __MyBase::operator --();
        return *this;
    }
    __Myt operator --(int){
        __Myt tmp(*this);
        --*this;
        return tmp;
    }
};

//class CBPlusTree
template<class Value>
struct __bplus_tree_node_base
{
    typedef Value   __ValType;
    //typedef Value * __ValType;
    static const U16 NODE_SZ = 4;  //items in a node; the min value is 4
    static const U16 HALF_NODE_SZ = NODE_SZ / 2;  //items
    __ValType   val_[NODE_SZ];
    U16         used_;
    U16         type_;  //0:leaf node; 1+:inner node
    bool Full() const{return used_ == NODE_SZ;}
    bool Less() const{return used_ < HALF_NODE_SZ;}
    bool IsLeaf() const{return type_ == 0;}
    const __ValType * First() const{return val_;}
    const __ValType * Last() const{return val_ + used_;}
    __ValType * First(){return val_;}
    __ValType * Last(){return val_ + used_;}
};

template<class Value>
struct __bplus_tree_inner_node : public __bplus_tree_node_base<Value>
{
    typedef __bplus_tree_node_base<Value>   __MyBase;
    typedef __bplus_tree_inner_node<Value>  __Myt;
    __MyBase *  ptr_[__MyBase::NODE_SZ + 1];
    __MyBase * const * FirstPtr() const{return ptr_;}
    __MyBase * const * LastPtr() const{return ptr_ + __MyBase::used_;}
    __MyBase ** FirstPtr(){return ptr_;}
    __MyBase ** LastPtr(){return ptr_ + __MyBase::used_ + 1;}
};

template<class Value>
struct  __bplus_tree_leaf_node : public __bplus_tree_node_base<Value>
{
    typedef __bplus_tree_node_base<Value>   __MyBase;
    typedef __bplus_tree_leaf_node<Value>   __Myt;
    __Myt *prev_,*next_;
};

template<class Value,class KeyOfValue,class Compare,class Alloc>
class __bplus_tree_const_iterator_impl
{
    typedef __bplus_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc> __Myt;
protected:
    typedef CBPlusTree<Value,KeyOfValue,Compare,Alloc>  __BPlusTree;
public:
    typedef std::bidirectional_iterator_tag         iterator_category;
    typedef typename __BPlusTree::value_type        value_type;
    typedef typename __BPlusTree::const_pointer     pointer;
    typedef typename __BPlusTree::const_reference   reference;
    typedef typename __BPlusTree::difference_type   difference_type;
protected:
    typedef __bplus_tree_leaf_node<value_type>  __Leaf;
public:
    __bplus_tree_const_iterator_impl():leaf_(0),pos_(0){}
    __bplus_tree_const_iterator_impl(__Leaf * node,size_t pos):leaf_(node),pos_(pos){}
    reference operator *() const    {return *leaf_->val_[pos_];}
    pointer operator ->() const     {return &(operator *());}
    bool operator ==(const __Myt & other) const {return leaf_ == other.leaf_ && pos_ == other.pos_;}
    bool operator !=(const __Myt & other) const {return !operator ==(other);}
    __Myt & operator ++(){
        if(++pos_ >= leaf_->used_){
            if(leaf_->next_){
                leaf_ = leaf_->next_;
                pos_ = 0;
            }else
                pos_ = leaf_->used_;
        }
        return *this;
    }
    __Myt operator ++(int){
        __Myt tmp(*this);
        ++*this;
        return tmp;
    }
    __Myt & operator --(){
        if(pos_){
            --pos_;
        }else if(leaf_->prev_){
            leaf_ = leaf_->prev_;
            pos_ = leaf_->used_ - 1;
        }
        return *this;
    }
    __Myt operator --(int){
        __Myt tmp(*this);
        --*this;
        return tmp;
    }
protected:
    __Leaf *    leaf_;
    size_t      pos_;
};

template<class Value,class KeyOfValue,class Compare, class Alloc>
class __bplus_tree_iterator_impl
    : public __bplus_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc>
{
    friend class CBPlusTree<Value,KeyOfValue,Compare,Alloc>;
    typedef __bplus_tree_const_iterator_impl<Value,KeyOfValue,Compare,Alloc>    __MyBase;
    typedef __bplus_tree_iterator_impl<Value,KeyOfValue,Compare,Alloc>          __Myt;
    typedef typename __MyBase::__BPlusTree  __BPlusTree;
    typedef typename __MyBase::__Leaf       __Leaf;
public:
    typedef std::bidirectional_iterator_tag     iterator_category;
    typedef typename __MyBase::value_type       value_type;
    typedef typename __MyBase::difference_type  difference_type;
    typedef typename __BPlusTree::pointer       pointer;
    typedef typename __BPlusTree::reference     reference;
    __bplus_tree_iterator_impl(){}
    __bplus_tree_iterator_impl(__Leaf * node,size_t pos):__MyBase(node,pos){}
    reference operator *() const    {return *__MyBase::leaf_->val_[__MyBase::pos_];}
    pointer operator ->() const     {return &(operator *());}
    __Myt & operator ++(){
        __MyBase::operator ++();
        return *this;
    }
    __Myt operator ++(int){
        __Myt tmp(*this);
        ++*this;
        return tmp;
    }
    __Myt & operator --(){
        __MyBase::operator --();
        return *this;
    }
    __Myt operator --(int){
        __Myt tmp(*this);
        --*this;
        return tmp;
    }
};

NS_IMPL_BEGIN

#endif

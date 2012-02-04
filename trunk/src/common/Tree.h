#ifndef DOZERG_RED_BLACK_TREE_H_20080114
#define DOZERG_RED_BLACK_TREE_H_20080114

/*
    STL风格的树
        CTree       红黑树
        CBPlusTree  B+树，还未完成
//*/

#include <cassert>
#include <utility>                  //std::pair,std::make_pair
#include <algorithm>                //std::swap,std::equal,std::lexicographical_compare,std::copy,std::copy_backward,std::lower_bound,std::upper_bound
#include <functional>               //std::less
#include <Tools.h>           //Tools::Construct,Tools::CIdentity,Tools::Destroy
#include <impl/Tree_impl.h>  //std::reverse_iterator,std::distance

NS_SERVER_BEGIN

template<
    class Value,
    class KeyOfValue = Tools::CIdentity<Value>,
    class Compare = std::less<typename KeyOfValue::result_type>,
    class Alloc = std::allocator<Value>
>class CTree{
    typedef CTree<Value,KeyOfValue,Compare,Alloc>       __Myt;
public:
    typedef Alloc                                       allocator_type;
    typedef Compare                                     key_compare;
    typedef KeyOfValue                                  key_of_value;
    typedef typename key_of_value::result_type          key_type;
    typedef typename allocator_type::value_type         value_type;
    typedef typename allocator_type::pointer            pointer;
    typedef typename allocator_type::const_pointer      const_pointer;
    typedef typename allocator_type::reference          reference;
    typedef typename allocator_type::const_reference    const_reference;
    typedef typename allocator_type::size_type          size_type;
    typedef typename allocator_type::difference_type    difference_type;
    typedef NS_IMPL::__rb_tree_iterator_impl<
        Value,key_of_value,Compare,Alloc>               iterator;
    typedef NS_IMPL::__rb_tree_const_iterator_impl<
        Value,key_of_value,Compare,Alloc>               const_iterator;
    typedef std::reverse_iterator<iterator>             reverse_iterator;
    typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;
protected:
    typedef typename iterator::__Node       __Node;
    typedef typename iterator::__NodePtr    __NodePtr;
    typedef typename iterator::__NodeAlloc  __NodeAlloc;
    typedef typename __Node::__NodeColor    __NodeColor;
    static __NodePtr & P(__NodePtr x)   {return x->parent_;}
    static __NodePtr & R(__NodePtr x)   {return x->right_;}
    static __NodePtr & L(__NodePtr x)   {return x->left_;}
    static __NodePtr & SL(__NodePtr x)  {return L(P(x));}
    static __NodePtr & SR(__NodePtr x)  {return R(P(x));}
    static bool isL(__NodePtr x)        {return x == L(P(x));}
    static bool isR(__NodePtr x)        {return x == R(P(x));}
    static bool isRed(__NodePtr x)      {return x && x->color_ == __Node::RED;}
    static bool isBlack(__NodePtr x)    {return !x || x->color_ == __Node::BLACK;}
    static value_type & V(__NodePtr x)  {return *x->data_ptr_;}
public:
    static size_type max_size(){return size_type(-1) / sizeof(value_type);}
    CTree(const key_compare & comp = key_compare()):comparator_(comp)    {construct_empty();}
    CTree(const __Myt & other):header_(0){
        __Myt tmp;
        tmp.fromSelf(other);
        swap(tmp);
    }
    ~CTree(){
        if(header_){
            clear();
            Tools::Destroy(header_,node_alloc_,false);
        }
    }
    size_type size() const  {return header_->size_;}
    bool empty() const      {return size() == 0;}
    iterator begin()        {return L(header_);}
    iterator end()          {return header_;}
    const_iterator begin() const    {return L(header_);}
    const_iterator end() const      {return header_;}
    reverse_iterator rbegin()       {return reverse_iterator(end());}
    reverse_iterator rend()         {return reverse_iterator(begin());}
    const_reverse_iterator rbegin() const   {return const_reverse_iterator(end());}
    const_reverse_iterator rend() const     {return const_reverse_iterator(begin());}
    key_compare key_comp() const            {return comparator_;}
    allocator_type get_allocator() const    {return data_alloc_;}
    iterator lower_bound(const key_type& key)               {return lower_bound_aux(key);}
    iterator upper_bound(const key_type& key)               {return upper_bound_aux(key);}
    const_iterator lower_bound(const key_type& key) const   {return lower_bound_aux(key);}
    const_iterator upper_bound(const key_type& key) const   {return upper_bound_aux(key);}
    void swap(__Myt & other) __DZ_NOTHROW{
        std::swap(header_,other.header_);
        std::swap(comparator_,other.comparator_);
    }
    __Myt & operator =(const __Myt & other){
        if(this != &other){
            __Myt tmp;
            tmp.fromSelf(other);
            swap(tmp);
        }
        return *this;
    }
    void clear() __DZ_NOTHROW{
        assert(header_);
        erase_tree(root());
        header_->size_ = 0;
        L(header_) = R(header_) = header_;
        P(header_) = 0;
    }
    iterator find(const key_type & key){
        const __NodePtr ret = lower_bound_aux(key);
        return (ret == header_ || comparator_(key,key_of_value()(V(ret))) ? header_ : ret);
    }
    const_iterator find(const key_type & key) const{
        const __NodePtr ret = lower_bound_aux(key);
        return (ret == header_ || comparator_(key,key_of_value()(V(ret))) ? header_ : ret);
    }
    size_type count(const key_type & key) const{
        std::pair<const_iterator,const_iterator> ret = equal_range(key);
        return std::distance(ret.first,ret.second);
    }
    std::pair<iterator,iterator> equal_range(const key_type & key){
        return std::make_pair(lower_bound_aux(key),upper_bound_aux(key));
    }
    std::pair<const_iterator,const_iterator> equal_range(const key_type & key) const{
        return std::make_pair(lower_bound_aux(key),upper_bound_aux(key));
    }
    iterator erase(iterator pos){
        assert(header_ && root() && pos.node_ptr_ != header_);
        const __NodePtr ret = pos.node_ptr_->next();
        erase_rebalance(pos.node_ptr_,ret);
        destroy_node(pos.node_ptr_);
        --header_->size_;
        return iterator(ret);
    }
    size_type erase(iterator first,iterator last){
        size_type sz;
        if(first == begin() && last == end()){
            sz = size();
            clear();
        }else
            for(sz = 0;first != last;++sz,erase(first++));
        return sz;
    }
    size_type erase(const key_type & key){
        std::pair<iterator,iterator> range = equal_range(key);
        return erase(range.first,range.second);
    }
    iterator insert_equal(const_reference value){
        assert(header_);
        if(empty())
            return insert_root(value);
        else{
            bool left_b = true;
            __NodePtr p = root();
            for(;;){
                if(compare_value(value,V(p))){
                    if(!L(p))
                        break;
                    p = L(p);
                }else{
                    if(!R(p)){
                        left_b = false;
                        break;
                    }
                    p = R(p);
                }
            }
            return insert_value(p,left_b,value);
        }
    }
    iterator insert_equal(iterator pos,const_reference value){
        assert(header_ && pos.node_ptr_);
        if(empty())
            return insert_root(value);
        else if(pos.node_ptr_ == L(header_)){
            if(!compare_value(*pos,value))                        //value <= smallest
                return insert_value(L(header_),true,value);
        }else if(pos.node_ptr_ == header_ || pos.node_ptr_ == R(header_))
            if(!compare_value(value,V(R(header_))))                //largest <= value
                return insert_value(R(header_),false,value);
        if(!compare_value(*pos,value)){
            iterator before = pos;
            --before;
            if(!compare_value(value,*before)){                    //*before <= value <= *pos
                if(!L(pos.node_ptr_))
                    return insert_value(pos.node_ptr_,true,value);
                else
                    return insert_value(before.node_ptr_,false,value);
            }
        }else if(!compare_value(value,*pos)){
            iterator after = pos;
            ++after;
            if(!compare_value(*after,value)){                    //*pos <= value <= *after
                if(!L(pos.node_ptr_))
                    return insert_value(pos.node_ptr_,false,value);
                else
                    return insert_value(after.node_ptr_,true,value);
            }
        }
        return insert_equal(value);
    }
    template <class InputIterator>
    void insert_equal(InputIterator first, InputIterator last){
        for(;first != last;++first)
            insert_equal(*first);
    }
    std::pair<iterator,bool> insert_unique(const_reference value){
        assert(header_);
        if(empty())
            return std::make_pair(insert_root(value),true);
        else{
            bool equal = false,left = true;
            __NodePtr p = root();
            for(;;){
                if(compare_value(value,V(p))){
                    if(!L(p))
                        break;
                    p = L(p);
                }else if(compare_value(V(p),value)){
                    if(!R(p)){
                        left = false;
                        break;
                    }
                    p = R(p);
                }else{
                    equal = true;
                    break;
                }
            }
            if(equal)
                return std::make_pair(p,false);
            else
                return std::make_pair(insert_value(p,left,value),true);
        }
    }
    iterator insert_unique(iterator pos,const_reference value){
        assert(header_ && pos.node_ptr_);
        if(empty())
            return insert_root(value);
        else if(pos.node_ptr_ == L(header_)){
            if(compare_value(value,*pos))           //smallest
                return insert_value(L(header_),true,value);
        }else if(pos.node_ptr_ == header_ || pos.node_ptr_ == R(header_))
            if(compare_value(V(R(header_)),value))  //largest
                return insert_value(R(header_),false,value);
        if(compare_value(value,*pos)){
            iterator before = pos;
            --before;
            if(compare_value(*before,value)){       //*before < value < *pos
                if(!L(pos.node_ptr_))
                    return insert_value(pos.node_ptr_,true,value);
                else
                    return insert_value(before.node_ptr_,false,value);
            }
        }else if(compare_value(*pos,value)){
            iterator after = pos;
            ++after;
            if(compare_value(value,*after)){        //*pos < value < *after
                if(!L(pos.node_ptr_))
                    return insert_value(pos.node_ptr_,false,value);
                else
                    return insert_value(after.node_ptr_,true,value);
            }
        }
        return insert_unique(value).first;
    }
    template <class InputIterator>
    void insert_unique(InputIterator first, InputIterator last){
        for(;first != last;++first)
            insert_unique(*first);
    }
    bool validate() const{
        if(isBlack(header_) || isRed(P(header_)))
            return false;
        if(P(header_)){
            if(SR(header_) && compare_value(V(SR(header_)),V(P(header_))))
                return false;
            if(SL(header_) && compare_value(V(P(header_)),V(SL(header_))))
                return false;
            if(!validate_node(SR(header_)) || !validate_node(SL(header_)))
                return false;
            __DZ_TRY{
                return (blackCount(SR(header_)) == blackCount(SL(header_)));
            }__DZ_CATCH_ALL{
                return false;
            }
        }
        return true;
    }
    bool operator ==(const __Myt & other) const{
        return size() == other.size() &&
            std::equal<const_iterator,const_iterator>(begin(),end(),other.begin());
    }
    bool operator <(const __Myt & other) const{
        return std::lexicographical_compare(begin(),end(),other.begin(),other.end());
    }
    bool operator !=(const __Myt & other) const {return !operator ==(other);}
    bool operator <=(const __Myt & other) const {return !(other < *this);}
    bool operator >(const __Myt & other) const  {return other < *this;}
    bool operator >=(const __Myt & other) const {return !(*this < other);}
protected:
    size_type blackCount(__NodePtr node) const{
        if(!node)
            return 1;
        const size_type c = blackCount(L(node));
        if(c != blackCount(R(node)))
            throw 0;
        return c + (isBlack(node) ? 1 : 0);
    }
    bool validate_node(__NodePtr node) const{
        if(node){
            if(isRed(node) && (isRed(L(node)) || isRed(R(node))))
                return false;
            if(R(node) && compare_value(V(R(node)),V(node)))
                return false;
            if(L(node) && compare_value(V(node),V(L(node))))
                return false;
            if(!validate_node(L(node)) || !validate_node(R(node)))
                return false;
        }
        return true;
    }
    void erase_rebalance(__NodePtr node,__NodePtr next){
        assert(header_ && node && node != header_);
        if(R(node) && L(node)){        //左,右子节点都不为空,那么必有后节点,作为替代节点,替代节点至多只有一个非空子节点
            assert(L(next) == 0 && ((isL(next) && P(next) != node) || (isR(next) && P(next) == node)));
            std::swap(node->color_,next->color_);
            next->leftSon(L(node));
            L(node) = 0;
            if(isL(next)){    //P(next) != node
                const __NodePtr p_next = P(next);
                root() == node ? setRoot(next) : (isL(node) ? P(node)->leftSon(next) : P(node)->rightSon(next));
                p_next->leftSon(node);
                const __NodePtr r_node = R(node);
                node->rightSon(R(next)),next->rightSon(r_node);
            }else{            //P(next) == node
                root() == node ? setRoot(next) : (isL(node) ? P(node)->leftSon(next) : P(node)->rightSon(next));
                node->rightSon(R(next)),next->rightSon(node);
            }
        }//此时node至多有一个非空子节点
        if(L(header_) == node)    //调整begin()和rbegin()
            L(header_) = next;
        if(R(header_) == node)
            R(header_) = node->prev();
        __NodePtr son = L(node) ? L(node) : R(node);    //son可能为0
        if(isBlack(node)){
            if(node != root()){
                __NodePtr p = P(node);
                bool left_b = isL(node);
                left_b ? p->leftSon(son) : p->rightSon(son);
                if(isBlack(son)){        //node和son都为黑,删除node会少一个黑节点
                    do{
                        __NodePtr sibling = (left_b ? R(p) : L(p));    //因为son这边少了一个黑节点,所以sibling必存在
                        assert(sibling);
                        if(isRed(sibling)){        //case 2    参考《红黑树.pdf》
                            singleRotate(sibling,p,left_b);
                            p->color_ = __Node::RED,sibling->color_ = __Node::BLACK;
                            sibling = (left_b ? R(p) : L(p));
                        }
                        if(isBlack(L(sibling)) && isBlack(R(sibling))){
                            if(isRed(p)){        //case 4
                                p->color_ = __Node::BLACK,sibling->color_ = __Node::RED;
                                break;
                            }else                //case 3
                                sibling->color_ = __Node::RED,son = p,p = P(p),left_b = isL(son);
                        }else{
                            if((left_b && isBlack(R(sibling))) || (!left_b && isBlack(L(sibling)))){    //case 5
                                const __NodePtr ss = (left_b ? L(sibling) : R(sibling));
                                singleRotate(ss,sibling,!left_b),ss->color_ = __Node::BLACK;
                                sibling->color_ = __Node::RED,sibling = ss;
                            }
                            singleRotate(sibling,p,left_b);        //case 6
                            sibling->color_ = p->color_,p->color_ = __Node::BLACK;
                            left_b ? R(sibling)->color_ = __Node::BLACK : L(sibling)->color_ = __Node::BLACK;
                            break;
                        }
                    }while(son != root());
                }else                    //node为黑,son为红,直接替换 
                    son->color_ = __Node::BLACK;
            }else    //node为root,直接替换
                setRoot(son);
        }else        //node为红,不可能是root,不论son什么颜色都可以直接替换
            isL(node) ? P(node)->leftSon(son) : P(node)->rightSon(son);
    }
    __NodePtr lower_bound_aux(const key_type & key) const{
        assert(header_);
        __NodePtr before = P(header_),after = header_;
        while(before){
            if(comparator_(key_of_value()(V(before)),key))
                before = R(before);
            else
                after = before,before = L(before);
        }
        return after;
    }
    __NodePtr upper_bound_aux(const key_type & key) const{
        assert(header_);
        __NodePtr before = P(header_),after = header_;
        while(before){
            if(comparator_(key,key_of_value()(V(before))))
                after = before,before = L(before);
            else
                before = R(before);
        }
        return after;
    }
    void fromSelf(const __Myt & other){
        assert(header_ && !root());
        if(!other.empty()){
            insert_root(V(P(other.header_)));
            copy_tree(root(),SL(other.header_),true);
            copy_tree(root(),SR(other.header_),false);
            L(header_) = root()->mostLeft();
            R(header_) = root()->mostRight();
            header_->size_ = other.size();
        }
    }
    void copy_tree(__NodePtr dst,__NodePtr src,bool left_b){
        assert(dst && dst != header_);
        if(src){
            const __NodePtr node = copy_node(src);
            left_b ? dst->leftSon(node) : dst->rightSon(node);
            copy_tree(node,L(src),true);
            copy_tree(node,R(src),false);
        }
    }
    void erase_tree(__NodePtr node) __DZ_NOTHROW{
        assert(node != header_);
        if(node){
            erase_tree(R(node));
            erase_tree(L(node));
            destroy_node(node);
        }
    }
    iterator insert_value(__NodePtr p,bool left_child,const_reference value){
        assert(p && p != header_);
        __NodePtr cur = create_node(value);
        if(left_child){
            p->leftSon(cur);
            if(L(header_) == p)
                L(header_) = cur;
        }else{
            p->rightSon(cur);
            if(R(header_) == p)
                R(header_) = cur;
        }
        if(isRed(p))    //rebalance for insert
            do{
                const bool left_p = isL(p);
                const __NodePtr uncle = (left_p ? SR(p) : SL(p));
                if(isRed(uncle)){
                    p->color_ = uncle->color_ = __Node::BLACK;
                    if(P(p) == root())
                        break;
                    cur = P(p);
                    p = P(cur);
                    cur->color_ = __Node::RED;
                }else{
                    if(left_p != isL(cur)){
                        singleRotate(cur,p,left_p);
                        std::swap(p,cur);
                    }
                    p->color_ = __Node::BLACK;
                    P(p)->color_ = __Node::RED;
                    singleRotate(p,P(p),!left_p);
                    break;
                }
            }while(isRed(p) && p != header_);
        ++header_->size_;
        return cur;
    }
    void singleRotate(__NodePtr node,__NodePtr p,bool left_d){
        assert(header_ && node && p);
        const __NodePtr gp = P(p);
        assert(gp);
        left_d ? p->rightSon(L(node)) : p->leftSon(R(node));
        if(gp == header_)
            setRoot(node);
        else if(isL(p))
            gp->leftSon(node);
        else
            gp->rightSon(node);
        left_d ? node->leftSon(p) : node->rightSon(p);
    }
    iterator insert_root(const_reference value){
        assert(header_ && !root());
        L(header_) = R(header_) = P(header_) = create_node(value);
        P(root()) = header_;
        root()->color_ = __Node::BLACK;
        header_->size_ = 1;
        return root();
    }
    __NodePtr copy_node(__NodePtr node) const{
        assert(node && node->data_ptr_);
        const __NodePtr ret = create_node(V(node));
        ret->color_ = node->color_;
        L(ret) = R(ret) = 0;
        return ret;
    }
    __NodePtr create_node(const_reference value){
        __NodePtr node = node_alloc_.allocate(1);
        node->data_ptr_ = 0;
        __DZ_TRY{
            node->data_ptr_ = data_alloc_.allocate(1);
            Tools::Construct(node->data_ptr_,value);
        }__DZ_CATCH_ALL{
            Tools::Destroy(node->data_ptr_,data_alloc_,false);
            Tools::Destroy(node,node_alloc_,false);
            __DZ_RETHROW;
        }
        node->color_ = __Node::RED;
        L(node) = R(node) = 0;
        return node;
    }
    void destroy_node(__NodePtr & node) __DZ_NOTHROW{
        if(node){
            Tools::Destroy(node->data_ptr_,data_alloc_);
            Tools::Destroy(node,node_alloc_,false);
        }
    }
    void construct_empty(){
        header_ = node_alloc_.allocate(1);
        header_->color_ = __Node::RED;
        P(header_) = 0;
        L(header_) = R(header_) = header_;
        header_->size_ = 0;
    }
    void setRoot(__NodePtr p) __DZ_NOTHROW{
        assert(header_);
        P(header_) = p;
        if(p){
            P(p) =  header_;
            p->color_ = __Node::BLACK;
        }
    }
    __NodePtr & root(){return P(header_);}
    bool compare_value(const_reference x,const_reference y) const{
        return comparator_(key_of_value()(x),key_of_value()(y));
    }
    //member fields
    __NodePtr       header_;
    allocator_type  data_alloc_;
    __NodeAlloc     node_alloc_;
    key_compare     comparator_;
};

template<class Value,class key_of_value,class Compare,class Alloc>
inline void swap(NS_SERVER::CTree<Value,key_of_value,Compare,Alloc> & left,NS_SERVER::CTree<Value,key_of_value,Compare,Alloc> & right){
    left.swap(right);
}

/*
    CTree NOTE:
        header_->size_ stores the size of CTree;
//*/

/*/class CBPlusTree
template<
    class Value,
    class KeyOfValue = Tools::CIdentity<Value>,
    class Compare = std::less<typename KeyOfValue::result_type>,
    class Alloc = std::allocator<Value>
>class CBPlusTree{
    //typedefs
    typedef CBPlusTree<Value,KeyOfValue,Compare,Alloc>  __Myt;
public:
    typedef Alloc                                       allocator_type;
    typedef Compare                                     key_compare;
    typedef KeyOfValue                                  key_of_value;
    typedef typename key_of_value::result_type          key_type;
    typedef typename allocator_type::value_type         value_type;
    typedef typename allocator_type::pointer            pointer;
    typedef typename allocator_type::const_pointer      const_pointer;
    typedef typename allocator_type::reference          reference;
    typedef typename allocator_type::const_reference    const_reference;
    typedef typename allocator_type::size_type          size_type;
    typedef typename allocator_type::difference_type    difference_type;
    typedef __bplus_tree_iterator_impl<
        Value,key_of_value,Compare,Alloc>               iterator;
    typedef __bplus_tree_const_iterator_impl<
        Value,key_of_value,Compare,Alloc>               const_iterator;
    typedef std::reverse_iterator<iterator>             reverse_iterator;
    typedef std::reverse_iterator<const_iterator>       const_reverse_iterator;
protected:
    typedef __bplus_tree_node_base<value_type>  __NodeBase;
    typedef __bplus_tree_inner_node<value_type> __InnerNode;
    typedef __bplus_tree_leaf_node<value_type>  __LeafNode;
    typedef typename __LeafNode::__ValType      __LeafVal;
    typedef typename allocator_type::
        template rebind<__InnerNode>::other     __InnerAlloc;
    typedef typename allocator_type::
        template rebind<__LeafNode>::other      __LeafAlloc;
    typedef std::pair<__LeafNode *,size_type>   __Pos;
    //key comparator helper used in lower/upper bound
    struct __comp{
        bool operator ()(const key_type & k1,const key_type & k2) const{
            return key_compare()(k1,k2);
        }
        bool operator ()(const_pointer p,const key_type & k) const{
            return operator ()(key_of_value()(*p),k);
        }
        bool operator ()(const key_type & k,iterator it) const{
            return operator ()(k,key_of_value()(*it));
        }
        bool operator ()(const key_type & k,const_pointer p) const{
            return operator ()(k,key_of_value()(*p));
        }
    };
    //leaf node __LeafVal type translator
    template<class Val>
    struct __valtype{
        const Val & operator()(const Val & val) const{
            return val;
        }
    };
    template<class Val>
    struct __valtype<Val *>{
        const Val * operator()(const Val & val) const{
            return &val;
        }
    };
    typedef __valtype<__LeafVal>    __LeafValTrans;
    //constants
    static const U16 HALF_NODE_SZ = __NodeBase::HALF_NODE_SZ;
public:
    //functions
    static size_type max_size(){return size_type(-1) / sizeof(value_type);}
    CBPlusTree(){construct_empty();}
    ~CBPlusTree(){}
    __Myt & operator =(const __Myt & other){
        return *this;
    }
    void clear(){
    }

    size_type size() const  {return size_;}
    bool empty() const      {return size() == 0;}
    iterator begin()        {return iterator(head_,0);}
    iterator end()          {return iterator(head_,head_->used_);}
    const_iterator begin() const    {return const_iterator(head_,0);}
    const_iterator end() const      {return const_iterator(head_,head_->used_);}
    reverse_iterator rbegin()       {return reverse_iterator(end());}
    reverse_iterator rend()         {return reverse_iterator(begin());}
    const_reverse_iterator rbegin() const   {return const_reverse_iterator(end());}
    const_reverse_iterator rend() const     {return const_reverse_iterator(begin());}
    key_compare key_comp() const            {return key_compare();}
    allocator_type get_allocator() const    {return allocator_type();}
    void swap(__Myt & other) __DZ_NOTHROW{
        std::swap(root_,other.root_);
        std::swap(head_,other.head_);
        std::swap(tail_,other.tail_);
        std::swap(size_,other.size_);
    }
    iterator lower_bound(const key_type & key){
        __Pos ret = lower_bound_aux(key);
        return iterator(ret.first,ret.second);
    }
    const_iterator lower_bound(const key_type & key) const{
        __Pos ret = lower_bound_aux(key);
        return const_iterator(ret.first,ret.second);
    }
    iterator upper_bound(const key_type & key){
        __Pos ret = upper_bound_aux(key);
        return iterator(ret.first,ret.second);
    }
    const_iterator upper_bound(const key_type & key) const{
        __Pos ret = upper_bound_aux(key);
        return const_iterator(ret.first,ret.second);
    }
    iterator find(const key_type & key){
        iterator ret = lower_bound(key);
        return (ret == end() || __comp()(key,ret) ? end() : ret);
    }
    const_iterator find(const key_type & key) const{
        const_iterator ret = lower_bound(key);
        return (ret == end() || __comp()(key,ret) ? end() : ret);
    }
    size_type count(const key_type & key) const{
        std::pair<const_iterator,const_iterator> ret = equal_range(key);
        return std::distance(ret.first,ret.second);
    }
    std::pair<iterator,iterator> equal_range(const key_type & key){
        return std::make_pair(lower_bound(key),upper_bound(key));
    }
    std::pair<const_iterator,const_iterator> equal_range(const key_type & key) const{
        return std::make_pair(lower_bound(key),upper_bound(key));
    }
    iterator insert_equal(const_reference value){
        bool unique = false;
        __Pos ret = insert_aux(value,unique);
        return iterator(ret.first,ret.second);
    }
    iterator insert_equal(iterator pos,const_reference value){
        return insert_equal(value);
    }
    template <class InputIterator>
    void insert_equal(InputIterator first, InputIterator last){
        for(;first != last;++first)
            insert_equal(*first);
    }
    std::pair<iterator,bool> insert_unique(const_reference value){
        bool unique = true;
        __Pos ret = insert_aux(value,unique);
        return std::make_pair(iterator(ret.first,ret.second),unique);
    }
    iterator insert_unique(iterator pos,const_reference value){
        return insert_unique(value).first;
    }
    template <class InputIterator>
    void insert_unique(InputIterator first, InputIterator last){
        for(;first != last;++first)
            insert_unique(*first);
    }
private:
    __Pos insert_aux(const_reference val,bool & unique){
        assert(root_);
        size_type pos = lower_bound_aux(root_,key_of_value()(val));
        __LeafVal split_key;
        __NodeBase * split_node = 0;
        __Pos ret;
        if(root_->IsLeaf())
            ret = insert_aux_leaf(leaf(root_),val,pos,split_key,split_node,unique);
        else
            ret = insert_aux_inner(inner(root_),val,pos,split_key,split_node,unique);
        if(split_node){ //need new root
            __InnerNode * new_node = getInner();
            new_node->type_ = root_->type_ + 1;
            insert_aux_inner_nofull(new_node,split_key,0,root_,split_node);
            root_ = new_node;
        }
        return ret;
    }
    __Pos insert_aux_inner(__InnerNode * node,const_reference val,size_type pos,
        __LeafVal & split_key,__NodeBase *& split_node,bool & unique)
    {
        assert(node && pos <= node->used_);
        __NodeBase * l_son = node->ptr_[pos];
        pos = lower_bound_aux(l_son,key_of_value()(val));
        __Pos ret;
        if(l_son->IsLeaf())
            ret = insert_aux_leaf(leaf(l_son),val,pos,split_key,split_node,unique);
        else
            ret = insert_aux_inner(inner(l_son),val,pos,split_key,split_node,unique);
        if(split_node){
            __NodeBase * r_son = split_node;
            split_node = 0;
            __LeafVal key = split_key;
            pos = lower_bound_aux(node,key_of_value()(val));
            if(node->Full())
                insert_aux_inner_full(node,key,pos,l_son,r_son,split_key,split_node);
            else
                insert_aux_inner_nofull(node,key,pos,l_son,r_son);
        }
        return ret;
    }
    void insert_aux_inner_full(__InnerNode * node,__LeafVal key,size_type pos,
        __NodeBase * l_son,__NodeBase * r_son,__LeafVal & split_key,__NodeBase *& split_node)
    {
        assert(node && l_son && r_son && node->Full());
        __InnerNode * new_node = getInner();        //new inner
        new_node->type_ = node->type_;
        split_node = new_node;
        split_key = node->val_[HALF_NODE_SZ - 1];   //middle key
        std::copy(node->First() + HALF_NODE_SZ,node->Last(),new_node->First());
        std::copy(node->FirstPtr() + HALF_NODE_SZ,node->LastPtr(),new_node->FirstPtr());
        node->used_ = HALF_NODE_SZ - 1;
        new_node->used_ = HALF_NODE_SZ;
        if(pos < HALF_NODE_SZ)
            insert_aux_inner_nofull(node,key,pos,l_son,r_son);
        else
            insert_aux_inner_nofull(new_node,key,pos - HALF_NODE_SZ,l_son,r_son);
    }
    void insert_aux_inner_nofull(__InnerNode * node,__LeafVal key,size_type pos,
        __NodeBase * l_son,__NodeBase * r_son)
    {
        assert(node && l_son && r_son && !node->Full());
        if(pos < node->used_){
            std::copy_backward(node->First() + pos,node->Last(),node->Last() + 1);
            std::copy_backward(node->FirstPtr() + pos,node->LastPtr(),node->LastPtr() + 1);
        }
        node->val_[pos] = key;
        node->ptr_[pos] = l_son;
        node->ptr_[pos + 1] = r_son;
        ++node->used_;
    }
    __Pos insert_aux_leaf(__LeafNode * node,const_reference val,size_type pos,
        __LeafVal & split_key,__NodeBase *& split_node,bool & unique)
    {
        assert(node && pos <= node->used_);
        if(unique && !__comp()(val,node->val_[pos])){   //equal key
            unique = false;
            return __Pos(node,pos);
        }else if(node->Full())
            return insert_aux_leaf_full(node,val,pos,split_key,split_node);
        return insert_aux_leaf_nofull(node,val,pos);
    }
    __Pos insert_aux_leaf_full(__LeafNode * node,const_reference val,size_type pos,
        __LeafVal & split_key,__NodeBase *& split_node)
    {
        assert(node && node->Full());
        __LeafNode * new_node = getLeaf();          //new leaf
        new_node->next_ = node->next_;
        new_node->prev_ = node;
        node->next_ = new_node;
        if(node == tail_){
            assert(!new_node->next_);
            tail_ = new_node;
        }else{
            assert(new_node->next_);
            new_node->next_->prev_ = new_node;
        }
        split_node = new_node;
        split_key = node->val_[HALF_NODE_SZ - 1];   //middle key
        std::copy(node->First() + HALF_NODE_SZ,node->Last(),new_node->First());
        node->used_ = new_node->used_ = HALF_NODE_SZ;
        if(pos < HALF_NODE_SZ)
            return insert_aux_leaf_nofull(node,val,pos);
        else
            return insert_aux_leaf_nofull(new_node,val,pos - HALF_NODE_SZ);
    }
    __Pos insert_aux_leaf_nofull(__LeafNode * node,const_reference val,size_type pos){
        assert(node && !node->Full());
        if(pos < node->used_)
            std::copy_backward(node->First() + pos,node->Last(),node->Last() + 1);
        node->val_[pos] = getValue(__LeafValTrans()(val));;
        ++node->used_;
        ++size_;
        return __Pos(node,pos);
    }
    __Pos lower_bound_aux(const key_type & key) const{
        assert(root_);
        for(__NodeBase * cur = root_;;){
            size_type p = lower_bound_aux(cur,key);
            if(cur->IsLeaf()){
                __LeafNode * lf = leaf(cur);
                assert(p != lf->used_ || !lf->next_);
                return __Pos(lf,p);
            }
            cur = inner(cur)->ptr_[p];
        }
    }
    size_type lower_bound_aux(__NodeBase * node,const key_type & key) const{
        assert(node);
        return std::lower_bound(node->First(),node->Last(),key,__comp()) - node->First();
    }
    __Pos upper_bound_aux(const key_type & key) const{
        assert(root_);
        for(__NodeBase * cur = root_;;){
            size_type p = upper_bound_aux(cur,key);
            if(cur->IsLeaf())
                return __Pos(leaf(cur),p);
            cur = inner(cur)->ptr_[p];
        }
    }
    size_type upper_bound_aux(__NodeBase * node,const key_type & key) const{
        assert(node);
        return std::upper_bound(node->First(),node->Last(),key,__comp()) - node->First();
    }
    void construct_empty(){
        root_ = tail_ = head_ = getLeaf();
        size_ = 0;
    }
    const_reference getValue(const_reference value) const{
        return value;
    }
    pointer getValue(const_pointer value) const{
        allocator_type alloc;
        pointer ret = alloc.allocate(1);
        __DZ_TRY{
            alloc.construct(ret,*value);
        }__DZ_CATCH_ALL{
            alloc.deallocate(ret,1);
            __DZ_RETHROW;
        }
        return ret;
    }
    __LeafNode * leaf(__NodeBase * node) const{
        assert(node->IsLeaf());
        return static_cast<__LeafNode *>(node);
    }
    __InnerNode * inner(__NodeBase * node) const{
        assert(!node->IsLeaf());
        return static_cast<__InnerNode *>(node);
    }
    __LeafNode * getLeaf() const{
        __LeafNode * ret = __LeafAlloc().allocate(1);
        memset(ret,0,sizeof(__LeafNode));
        return ret;
    }
    __InnerNode * getInner() const{
        __InnerNode * ret = __InnerAlloc().allocate(1);
        memset(ret,0,sizeof(__InnerNode));
        ret->type_ = 1;
        return ret;
    }
    //member fields
    __NodeBase *    root_;
    __LeafNode *    head_;
    __LeafNode *    tail_;
    size_type       size_;
};

template<class Value,class key_of_value,class Compare,class Alloc>
inline void swap(NS_SERVER::CBPlusTree<Value,key_of_value,Compare,Alloc> & left,NS_SERVER::CBPlusTree<Value,key_of_value,Compare,Alloc> & right){
    left.swap(right);
}
//*/

NS_SERVER_END

#endif

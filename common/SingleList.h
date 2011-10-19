#ifndef DOZERG_SINGLE_LIST_H_20070917
#define DOZERG_SINGLE_LIST_H_20070917

/*
    STL风格单向链表,主要实现push_back,front,pop_front,size函数
    除了erase_after(iterator,iterator)
    和insert_after(iterator,size_type,const_reference)
    复杂度为O(n)外,其余函数复杂度均为O(1)
        CSingleList
    History:
        20080520    修正insertChainAfter里插入节点后未更改phead_->tail_的bug
                    修正eraseNodeAfter和eraseChainAfter里未更改phead_->tail_的bug
        20080912    增加append函数
        20080917    增加copy构造，赋值等函数，完成应有的接口
//*/

#include <common/Tools.h>   //Tools::CInterTypeTag,Tools::Destroy
#include <common/impl/SingleList_impl.h>

NS_SERVER_BEGIN

template<class T,class Alloc = __DZ_ALLOC<T> >
class CSingleList
{
//typedefs:
    typedef CSingleList<T,Alloc>    __Myt;
public:
    typedef T               value_type;
    typedef T &             reference;
    typedef const T &       const_reference;
    typedef T *             pointer;
    typedef const T *       const_pointer;
    typedef size_t          size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef typename Alloc::
        template rebind<T>::other               allocator_type;
    typedef NS_IMPL::__slist_const_iterator<T>  const_iterator;
    typedef NS_IMPL::__slist_iterator<T,Alloc>  iterator;
private:
    typedef typename iterator::__node       __node;
    typedef typename iterator::__node_ptr   __node_ptr;
    typedef typename Alloc::
        template rebind<__node>::other      __node_alloc;
//functions:
public:
    explicit CSingleList(allocator_type = allocator_type())
        : phead_(0)
    {
        init();
    }
    explicit CSingleList(size_type sz,const_reference value = value_type())
        : phead_(0)
    {
        init();
        insert_after(phead_->tail_,sz,value);
    }
    CSingleList(const __Myt & a)
        : phead_(0)
    {
        init();
        insert_after(phead_->tail_,a.begin(),a.end());
    }
    template<class InputIter>
    CSingleList(InputIter first,InputIter last)
        : phead_(0)
    {
        init();
        insert_after(phead_->tail_,first,last);
    }
    ~CSingleList()                  {destroy();}
    iterator begin()                {return phead_->next_;}
    iterator end()                  {return 0;}
    const_iterator begin() const    {return phead_->next_;}
    const_iterator end() const      {return 0;}
    reference front()               {return *begin();}
    reference back()                {return *phead_->tail_->pdata_;}
    const_reference front() const   {return *begin();}
    const_reference back() const    {return *phead_->tail_->pdata_;}
    bool empty() const              {return size_ == 0;}
    size_type size() const          {return size_;}
    void clear() __DZ_NOTHROW       {eraseChainAfter(phead_,0);}
    allocator_type get_allocator() const        {return allocator_type();}
    void push_front(const_reference v)          {insert_after(phead_,v);}
    void push_back(const_reference v)           {insert_after(phead_->tail_,v);}
    void pop_front() __DZ_NOTHROW               {eraseNodeAfter(phead_);}   //no pop_back()
    void erase_after(iterator pos) __DZ_NOTHROW {eraseNodeAfter(pos.ptr_);}
    void erase_after(iterator before_first,iterator last) __DZ_NOTHROW{  //erase range [before_first->next,last)
        eraseChainAfter(before_first.ptr_,last.ptr_);
    }
    iterator insert_after(iterator pos,const_reference v){
        __node_ptr node = createNode(v);
        insertNodeAfter(pos.ptr_,node);
        return node;
    }
    void insert_after(iterator pos,size_type elemSz,const_reference v){
        __node_ptr head,tail;
        createChainFill(head,tail,elemSz,v);
        insertChainAfter(pos.ptr_,head,tail,elemSz);
    }
    template<class InputIter>
    void insert_after(iterator pos,InputIter first,InputIter last){
        typedef typename std::iterator_traits<InputIter>::iterator_category __IterCategory;
        insertAfterAux(pos,first,last,__IterCategory());
    }
    void assign(size_type elemSz,const_reference value){
        if(elemSz == 0){
            clear();
        }else if(elemSz < size()){
            iterator befor_first = std::fill_n(begin(),elemSz - 1,value);
            *befor_first = value;
            erase_after(befor_first,end());
        }else{
            if(size() > 0)
                std::fill(begin(),end(),value);
            insert_after(phead_->tail_,elemSz - size(),value);
        }
    }
    template<class InputIter>
    void assign(InputIter first,InputIter last){
        typedef typename std::iterator_traits<InputIter>::iterator_category __IterCategory;
        assignAux(first,last,__IterCategory());
    }
    void resize(size_type sz,const_reference value = value_type()){
        if(sz < size()){
            iterator befor_first = begin();
            for(;--sz > 0;++befor_first);
            erase_after(befor_first,end());
        }else
            insert_after(phead_->tail_,sz - size(),value);
    }
    __Myt & operator =(const __Myt & a){
        if(this != &a)
            assignAux(a.begin(),a.end()
            ,typename std::iterator_traits<iterator>::iterator_category());
        return *this;
    }
    void swap(__Myt & a) __DZ_NOTHROW{
        std::swap(size_,a.size_);
        std::swap(phead_,a.phead_);
    }
    void append(__Myt & a) __DZ_NOTHROW{
        if(!a.empty()){
            insertChainAfter(phead_->tail_,a.phead_->next_,a.phead_->tail_,a.size());
            a.init();
        }
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
private:
    template<class IntegerType>
    void assignAux(IntegerType sz,IntegerType val,Tools::CInterTypeTag){
        assign(size_type(sz),value_type(val));
    }
    template<class InputIter>
    void assignAux(InputIter first,InputIter last,std::input_iterator_tag){
        if(first == last){
            clear();
        }else{
            if(empty()){
                insertAfterAux(phead_->tail_,first,last,std::input_iterator_tag());
            }else{
                iterator cur = begin(),befor_first = cur;
                *cur++ = *first++;
                for(;first != last && cur != end();++befor_first)
                    *cur++ = *first++;
                if(cur == end()){
                    insertAfterAux(phead_->tail_,first,last,std::input_iterator_tag());
                }else
                    erase_after(befor_first,end());
            }
        }
    }
    template<class IntegerType>
    void insertAfterAux(iterator pos,IntegerType sz,IntegerType val,Tools::CInterTypeTag){
        insert_after(pos,size_type(sz),value_type(val));
    }
    template<class InputIter>
    void insertAfterAux(iterator pos,InputIter first,InputIter last,std::input_iterator_tag){
        if(first != last){
            __node_ptr head,tail;
            size_type sz = createChainCopy(head,tail,first,last);
            insertChainAfter(pos.ptr_,head,tail,sz);
        }
    }

    void eraseNodeAfter(__node_ptr pos){
        __node_ptr cur = pos->next_;
        if(cur){
            pos->next_ = cur->next_;
            if(phead_->tail_ == cur)
                phead_->tail_ = pos;
            --size_;
            destroyNode(cur);
        }
    }
    void eraseChainAfter(__node_ptr pos,__node_ptr last){
        __node_ptr first = pos->next_;
        pos->next_ = last;
        if(!last)
            phead_->tail_ = pos;
        size_ -= destroyChain(first,last);
    }
    template<class InputIter>
    size_type createChainCopy(__node_ptr & head,__node_ptr & tail,InputIter first,InputIter last) const{
        size_type ret = 0;
        head = tail = 0;
        for(__node_ptr cur = 0;first != last;++ret,++first){
            __DZ_TRY{
                cur = createNode(*first);
            }__DZ_CATCH_ALL{
                destroyChain(head,0);
                __DZ_RETHROW;
            }
            cur->next_ = 0;
            if(!head)
                head = cur;
            if(tail)
                tail->next_ = cur;
            tail = cur;
        }
        return ret;
    }
    void createChainFill(__node_ptr & head,__node_ptr & tail,size_type elemSz,const_reference v) const{
        if(elemSz > 0){
            tail = head = createNode(v);
            __DZ_TRY{
                for(tail->next_ = 0;--elemSz > 0;tail = tail->next_,tail->next_ = 0)
                    tail->next_ = createNode(v);
            }__DZ_CATCH_ALL{
                destroyChain(head,0);
                __DZ_RETHROW;
            }
        }
    }
    size_type destroyChain(__node_ptr first,__node_ptr last) const __DZ_NOTHROW{
        size_type ret = 0;
        for(;first != last;++ret){
            __node_ptr cur = first;
            first = first->next_;
            destroyNode(cur);
        }
        return ret;
    }
    void insertNodeAfter(__node_ptr pos,__node_ptr node) __DZ_NOTHROW{
        insertChainAfter(pos,node,node,1);
    }
    void insertChainAfter(__node_ptr pos,__node_ptr head,__node_ptr tail,size_type sz) __DZ_NOTHROW{
        tail->next_ = pos->next_;
        pos->next_ = head;
        if(!tail->next_)
            phead_->tail_ = tail;
        size_ += sz;
    }
    __node_ptr createNode(const_reference v) const{
        __node_ptr ret = __node_alloc().allocate(1);
        ret->pdata_ = 0;
        __DZ_TRY{
            ret->pdata_ = allocator_type().allocate(1);
            allocator_type().construct(ret->pdata_,v);
        }__DZ_CATCH_ALL{
            Tools::Destroy(ret->pdata_,allocator_type(),false);
            Tools::Destroy(ret,__node_alloc(),false);
            __DZ_RETHROW;
        }
        return ret;
    }
    void destroyNode(__node_ptr & node) const __DZ_NOTHROW{
        if(node){
            Tools::Destroy(node->pdata_,allocator_type());
            Tools::Destroy(node,__node_alloc(),false);
        }
    }
    void init(){
        if(!phead_)
            phead_ = __node_alloc().allocate(1);
        phead_->next_ = 0;
        phead_->tail_ = phead_;
        size_ = 0;
    }
    void destroy() __DZ_NOTHROW{
        clear();
        Tools::Destroy(phead_,__node_alloc(),false);
    }
//fields:
    __node *    phead_;    //链表头节点
    size_type   size_;
};

NS_SERVER_END

namespace std{
    template<class T,class Alloc>
    void swap(NS_SERVER::CSingleList<T,Alloc> & a,NS_SERVER::CSingleList<T,Alloc> & b)
    {
        a.swap(b);
    }
}//namespace std

#endif

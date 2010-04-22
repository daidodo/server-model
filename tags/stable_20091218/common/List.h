#ifndef DOZERG_LIST_H_20071219
#define DOZERG_LIST_H_20071219

/*
    STL风格的双向链表
    主要改进是size()为O(1)
        CList
    History:
        20080917    修正resize()一个bug
        20081006    将全局swap函数加入std名字空间
//*/

#include <algorithm>
#include <assert.h>
#include <common/Tools.h>
#include <common/impl/List_impl.h>

NS_SERVER_BEGIN

template<class T,class Alloc = __DZ_ALLOC<T> >
class CList
{
    typedef CList<T,Alloc>          __Myt;
    typedef NS_IMPL::__list_node<T> __NodeType;
    typedef __NodeType *            __NodePtr;
public:
    typedef T               value_type;
    typedef T &             reference;
    typedef const T &       const_reference;
    typedef T *             pointer;
    typedef const T *       const_pointer;
    typedef size_t          size_type;
    typedef std::ptrdiff_t  difference_type;
    typedef typename Alloc::
        template rebind<T>::other                   allocator_type;
    typedef NS_IMPL::__list_iterator_impl<T,Alloc>  iterator;
    typedef NS_IMPL::__list_const_iterator_impl<T>  const_iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    static size_type max_size(){return size_type(-1) / sizeof(__NodePtr);}
    explicit CList(allocator_type = allocator_type()){initialize_empty();}
    explicit CList(size_type count,const_reference value = value_type(),
        allocator_type = allocator_type())
    {
        initialize_fill(count,value);
    }
    CList(const __Myt & other){
        initialize_copy(other.begin(),other.end(),std::bidirectional_iterator_tag());
    }
    template<class InputIterator>
    CList(InputIterator first,InputIterator last,allocator_type = allocator_type()){
        typedef typename std::iterator_traits<InputIterator>::iterator_category __IterCategory;
        initialize_copy(first,last,__IterCategory());
    }
    ~CList()                        {cleanup();}
    iterator begin()                {return end_->next;}
    iterator end()                  {return end_;}
    const_iterator begin() const    {return end_->next;}
    const_iterator end() const      {return end_;}
    reverse_iterator rbegin()       {return reverse_iterator(end());}
    reverse_iterator rend()         {return reverse_iterator(begin());}
    const_reverse_iterator rbegin() const   {return const_reverse_iterator(end());}
    const_reverse_iterator rend() const     {return const_reverse_iterator(begin());}
    allocator_type get_allocator() const    {return __DataAlloc();}
    reference front()                       {return *begin();}
    reference back()                        {return *iterator(end_->prev);}
    const_reference front() const           {return *begin();}
    const_reference back() const            {return *iterator(end_->prev);}
    bool empty() const                      {return end_->next == end_;}
    size_type size() const                  {return end_->size_;}
    void clear() __DZ_NOTHROW               {erase(begin(),end());}
    void push_front(const_reference value)  {insert(begin(),value);}
    void push_back(const_reference value)   {insert(end_,value);}
    void pop_front() __DZ_NOTHROW{
        assert(size() > 0);
        erase(begin());
    }
    void pop_back() __DZ_NOTHROW{
        assert(size() > 0);
        erase(end_->prev);
    }
    __Myt & operator =(const __Myt & other){
        if(this != &other)
            assign_aux(other.begin(),other.end(),
            typename std::iterator_traits<iterator>::iterator_category());
        return *this;
    }
    void swap(__Myt & other){
        std::swap(end_,other.end_);
    }
    iterator insert(iterator pos,const_reference value){
        const __NodePtr ret = create_node(value);
        insert_node(pos.node_,ret);
        return ret;
    }
    void insert(iterator pos,size_type sz,const_reference value){
        if(sz > 0){
            __NodePtr head,tail;
            create_chain_fill(head,tail,sz,value);
            insert_chain(pos.node_,head,tail,sz);
        }
    }
    template<class InputIterator>
    void insert(iterator pos,InputIterator first,InputIterator last){
        typedef typename std::iterator_traits<InputIterator>::iterator_category __IterCategory;
        insert_aux(pos,first,last,__IterCategory());
    }
    iterator erase(iterator pos) __DZ_NOTHROW{
        assert(pos != end());
        const __NodePtr ret = pos.node_->next;
        pos.node_->prev->next = ret;
        ret->prev = pos.node_->prev;
        destroy_node(pos.node_);
        --end_->size_;
        return ret;
    }
    iterator erase(iterator first,iterator last) __DZ_NOTHROW{
        if(first != last){
            const __NodePtr tail = last.node_->prev;
            first.node_->prev->next = last.node_;
            last.node_->prev = first.node_->prev;
            end_->size_ -= destroy_chain(first.node_,tail);
        }
        return last;
    }
    void assign(size_type count,const_reference value){
        if(count == 0){
            clear();
        }else if(count < size()){
            const iterator left = std::fill_n(begin(),count,value);
            erase(left,end_);
        }else{
            if(size() > 0)
                std::fill(begin(),end(),value);
            insert(end(),count - size(),value);
        }
    }
    template<class InputIterator>
    void assign(InputIterator first,InputIterator last){
        typedef typename std::iterator_traits<InputIterator>::iterator_category __IterCategory;
        assign_aux(first,last,__IterCategory());
    }
    void resize(size_type newSz,const_reference value = value_type()){
        if(newSz < (size() >> 1)){
            iterator cur = begin();
            for(;newSz > 0;--newSz,++cur);
            erase(cur,end());
        }else if(newSz < size()){
            iterator cur = end();
            for(;newSz < size();++newSz,--cur);
            erase(cur,end());
        }else
            insert(end(),newSz - size(),value);
    }
    void reverse(){
        if(size() > 1){
            const __NodePtr first = end_->next;
            __NodePtr cur = first;
            do{
                const __NodePtr tmp = cur->next;
                cur->next = cur->prev;
                cur->prev = tmp;
                cur = tmp;
            }while(cur != end_);
            end_->next = end_->prev;
            end_->prev = first;
        }
    }
    void remove(const_reference value){
        for(iterator cur = begin();cur.node_ != end_;){
            if(value == *cur){
                const iterator head = cur;
                while((++cur).node_ != end_ && value == *cur);
                erase(head,cur);
            }else
                ++cur;
        }
    }
    template<class Predicate>
    void remove_if(Predicate pred){
        for(iterator cur = begin();cur.node_ != end_;){
            if(pred(*cur)){
                const iterator head = cur;
                while((++cur).node_ != end_ && pred(*cur));
                erase(head,cur);
            }else
                ++cur;
        }
    }
    void unique(){
        if(size() > 1){
            iterator cur = begin(),next = cur.node_->next;
            do{
                if(*cur == *next)
                    for(++next;next != end_ && *cur == *next;++next);
                erase(++cur,next);
                cur = next++;
            }while(cur != end_ && next != end_);
        }
    }
    template<class BinaryPredicate>
    void unique(BinaryPredicate pred){
        if(size() > 1){
            iterator cur = begin(),next = cur.node_->next;
            do{
                if(pred(*cur,*next))
                    for(++next;next != end_ && pred(*cur,*next);++next);
                erase(++cur,next);
                cur = next++;
            }while(cur != end_ && next != end_);
        }
    }
    void merge(__Myt & other){
        if(other.size() > 0){
            size_type sz = 0;
            iterator cur1 = begin(),cur2 = other.begin();
            for(;cur1.node_ != end_ && cur2.node_ != other.end_;++cur1){
                if(*cur2 < *cur1){
                    const iterator head = cur2;
                    size_type cur_sz = 1;
                    while((++cur2).node_ != other.end_ && !(*cur1 < *cur2))
                        ++cur_sz;
                    splice(cur1,other,head,cur2,cur_sz);
                    sz += cur_sz;
                }
            }
            if(cur2.node_ != other.end_)
                splice(end_,other);
        }
    }
    template<class StrictWeakOrdering>
    void merge(__Myt & other,StrictWeakOrdering comp){
        if(other.size() > 0){
            size_type sz = 0;
            iterator cur1 = begin(),cur2 = other.begin();
            for(;cur1.node_ != end_ && cur2.node_ != other.end_;++cur1){
                if(comp(*cur2,*cur1)){
                    const iterator head = cur2;
                    size_type cur_sz = 1;
                    while((++cur2).node_ != other.end_ && !comp(*cur1,*cur2))
                        ++cur_sz;
                    splice(cur1,other,head,cur2,cur_sz);
                    sz += cur_sz;
                }
            }
            if(cur2.node_ != other.end_)
                splice(end_,other);
        }
    }
    void splice(iterator pos,__Myt & other){
        if(other.size() > 0)
            transfer(pos.node_,other.end_->next,other.end_,other.size());
        other.end_->size_ = 0;
    }
    void splice(iterator pos,__Myt & other,iterator iter){
        if(iter != pos && iter.node_->next != pos.node_)
            transfer(pos.node_,iter.node_,iter.node_->next,1);
        --other.end_->size_;
    }
    void splice(iterator pos,__Myt & other,iterator first,iterator last,size_type sz = 0){
        if(first != last){
            if(sz == 0)
                sz = std::distance(first,last); //this is needed because we have end_->size_ member field
            transfer(pos.node_,first.node_,last.node_,sz);
            other.end_->size_ -= sz;
        }
    }
    void sort(){
        //copy from VC8 STL, merge sorting
        if(size() > 1){
            const size_t MAX_HELPERS = 25;
            __Myt tmp_list(get_allocator()), help_list[MAX_HELPERS + 1];
            size_t all_used = 0;
            while (!empty())
            {   // sort another element, using bins
                tmp_list.splice(tmp_list.begin(),*this,begin());
                size_t cur_use = 0;
                for(;cur_use < all_used && !help_list[cur_use].empty();++cur_use){
                    // merge into ever larger bins
                    help_list[cur_use].merge(tmp_list);
                    help_list[cur_use].swap(tmp_list);
                }
                if (cur_use == MAX_HELPERS)
                    help_list[cur_use - 1].merge(tmp_list);
                else{   // spill to new bin, while they last
                    help_list[cur_use].swap(tmp_list);
                    if (cur_use == all_used)
                        ++all_used;
                }
            }
            for(size_t cur_use = 1;cur_use < all_used;++cur_use)
                help_list[cur_use].merge(help_list[cur_use - 1]);   // merge up
            splice(begin(),help_list[all_used - 1]);    // result in last bin
        }
    }
    template <class StrictWeakOrdering>
    void sort(StrictWeakOrdering comp){
        //copy from VC8 STL
        if(size() > 1){
            const size_t MAX_HELPERS = 25;
            __Myt tmp_list(get_allocator()), help_list[MAX_HELPERS + 1];
            size_t all_used = 0;
            while (!empty())
            {   // sort another element, using bins
                tmp_list.splice(tmp_list.begin(),*this,begin());
                size_t cur_use = 0;
                for(;cur_use < all_used && !help_list[cur_use].empty();++cur_use){
                    // merge into ever larger bins
                    help_list[cur_use].merge(tmp_list,comp);
                    help_list[cur_use].swap(tmp_list);
                }
                if (cur_use == MAX_HELPERS)
                    help_list[cur_use - 1].merge(tmp_list,comp);
                else{   // spill to new bin, while they last
                    help_list[cur_use].swap(tmp_list);
                    if (cur_use == all_used)
                        ++all_used;
                }
            }
            for(size_t cur_use = 1;cur_use < all_used;++cur_use)
                help_list[cur_use].merge(help_list[cur_use - 1],comp);  // merge up
            splice(begin(),help_list[all_used - 1]);    // result in last bin
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
    //put [first,last) in front of pos,first < last && (pos < first || pos > last)
    void transfer(__NodePtr pos,__NodePtr first,__NodePtr last,size_type sz) __DZ_NOTHROW{
        first->prev->next = last;
        last = last->prev;
        last->next->prev = first->prev;
        insert_chain(pos,first,last,sz);
    }
    template<class IntegerType>
    void assign_aux(IntegerType sz,IntegerType val,Tools::CInterTypeTag){
        assign(size_type(sz),value_type(val));
    }
    template<class InputIterator>
    void assign_aux(InputIterator first,InputIterator last,std::input_iterator_tag){
        if(first != last){
            if(size() == 0){
                insert_aux(end_,first,last,std::input_iterator_tag());
            }else{
                std::pair<InputIterator,iterator> left = 
                    Tools::SafeCopy(first,last,begin(),size());
                if(left.second == end()){
                    if(left.first != last){
                        __NodePtr head,tail;
                        const size_type sz = create_chain_copy(head,tail,left.first,last);
                        insert_chain(end_,head,tail,sz);
                    }
                }else
                    erase(left.second,end());
            }
        }
    }
    template<class IntegerType>
    void insert_aux(iterator pos,IntegerType sz,IntegerType val,Tools::CInterTypeTag){
        insert(pos,size_type(sz),value_type(val));
    }
    template<class InputIterator>
    void insert_aux(iterator pos,InputIterator first,InputIterator last,std::input_iterator_tag){
        if(first != last){
            __NodePtr head,tail;
            size_type sz = create_chain_copy(head,tail,first,last);
            insert_chain(pos.node_,head,tail,sz);
        }
    }
    template<class InputIterator>
    size_type create_chain_copy(__NodePtr & head,__NodePtr & tail,
        InputIterator first,InputIterator last){    //first < last,return distance(first,last)
            tail = head = create_node(*first++);
            size_type ret = 1;
            if(first != last){
                __DZ_TRY{
                    do{
                        __NodePtr cur = create_node(*first++);
                        tail->next = cur;
                        cur->prev = tail;
                        tail = cur;
                        ++ret;
                    }while(first != last);
                }__DZ_CATCH_ALL{
                    destroy_chain(head,tail);
                    __DZ_RETHROW;
                }
            }
            return ret;
    }
    void create_chain_fill(__NodePtr & head,__NodePtr & tail,size_type sz,
        const_reference value) const    //sz >= 1
    {
        tail = head = create_node(value);
        if(--sz > 0){
            __DZ_TRY{
                do{
                    __NodePtr cur = create_node(value);
                    tail->next = cur;
                    cur->prev = tail;
                    tail = cur;
                }while(--sz > 0);
            }__DZ_CATCH_ALL{
                destroy_chain(head,tail);
                __DZ_RETHROW;
            }
        }
    }
    size_type destroy_chain(__NodePtr head,__NodePtr tail) const __DZ_NOTHROW{  //head <= tail,return the number of elements destroyed
        size_type ret = 1;
        while(head != tail){
            assert(head != end_);
            __NodePtr cur = head;
            head = head->next;
            destroy_node(cur);
            ++ret;
        }
        assert(head != end_);
        destroy_node(head);
        return ret;
    }
    void cleanup() __DZ_NOTHROW{
        clear();
        Tools::Destroy(end_,__NodeAlloc(),false);
    }
    void insert_node(__NodePtr pos,__NodePtr node_) __DZ_NOTHROW{
        insert_chain(pos,node_,node_,1);
    }
    void insert_chain(__NodePtr pos,__NodePtr head,__NodePtr tail,size_type sz) __DZ_NOTHROW{   //sz = distance(head,tail),head <= tail
        tail->next = pos;
        head->prev = pos->prev;
        head->prev->next = head;
        pos->prev = tail;
        end_->size_ += sz;
    }
    void initialize_empty(){
        end_ = __NodeAlloc().allocate(1);
        end_->next = end_->prev = end_;
        end_->size_ = 0;
    }
    template<class IntegerType>
    void initialize_copy(IntegerType sz,IntegerType val,Tools::CInterTypeTag){
        initialize_fill(size_type(sz),value_type(val));
    }
    template<class InputIterator>
    void initialize_copy(InputIterator first,InputIterator last,std::input_iterator_tag){
        initialize_empty();
        insert_aux(end_,first,last,std::input_iterator_tag());
    }
    void initialize_fill(size_type sz,const_reference value){
        initialize_empty();
        __DZ_TRY{
            insert(end_,sz,value);
        }__DZ_CATCH_ALL{
            cleanup();
            __DZ_RETHROW;
        }
    }
    __NodePtr create_node(const_reference val) const{
        __NodePtr node = __NodeAlloc().allocate(1);
        node->data_ptr = 0;
        __DZ_TRY{
            node->data_ptr = __DataAlloc().allocate(1);
            Tools::Construct(node->data_ptr,val);
        }__DZ_CATCH_ALL{
            Tools::Destroy(node->data_ptr,__DataAlloc(),false);
            Tools::Destroy(node,__NodeAlloc(),false);
            __DZ_RETHROW;
        }
        return node;
    }
    void destroy_node(__NodePtr & node) const __DZ_NOTHROW{
        if(node){
            Tools::Destroy(node->data_ptr,__DataAlloc());
            Tools::Destroy(node,__NodeAlloc(),false);
        }
    }
    typedef allocator_type                  __DataAlloc;
    typedef typename __DataAlloc::
        template rebind<__NodeType>::other  __NodeAlloc;
    //member fields
    __NodePtr       end_;
};

NS_SERVER_END

namespace std{
    template<class T,class A>
    inline void swap(NS_SERVER::CList<T,A> & left,NS_SERVER::CList<T,A> & right){
        left.swap(right);
    }
}//namespace std

/*
    NOTE: 
        end_->size_ is used as VC8 does
        既然CList的end_->data域没有用，应该节省下来，所以我给__list_node的data类型申明为T*，并用end_->size_作为union
        CList里有data_alloc和node_alloc这2个allocator
        加const
        operator >,<,==,!=,>=,<=可以实现为成员函数
//*/

#endif

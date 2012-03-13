#ifndef DOZERG_REFERENCE_COUNT_POINTER_IMPL_H_20080226
#define DOZERG_REFERENCE_COUNT_POINTER_IMPL_H_20080226

/*
    CSharedPtr的内部实现
        _ref_imp
    History
        20080605    在_ref_imp::subRef添加p = 0,修正重复删除bug
        20080827    通过__alloc得到__elem_alloc
        20120118    增加release功能
//*/

#include <Tools.h>   //Tools::Destroy

NS_IMPL_BEGIN

template<class Elem,class IntType>
struct _ref_imp{
    typedef typename Elem::allocator_type   __alloc;
    typedef typename __alloc::
        template rebind<Elem>::other        __elem_alloc;
    typedef typename __alloc::
        template rebind<_ref_imp>::other    __ref_alloc;
    static _ref_imp * GetObject(Elem * pe){
        _ref_imp * ret = __ref_alloc().allocate(1);
        return new (ret) _ref_imp(pe);
    }
    explicit _ref_imp(Elem * pe){
        ptr_ = pe;
        cnt_ = 1;
    }
    void addRef(_ref_imp *& p){
        p = this;
        ++cnt_;
    }
    static void changeRef(_ref_imp *& p,_ref_imp * v){
        if(p != v){
            subRef(p);
            if(v)
                v->addRef(p);
        }
    }
    static void subRef(_ref_imp *& p, bool release = false){
        if(p){
            if(!--(p->cnt_)){
                if(!release)
                    Tools::Destroy(p->ptr_,__elem_alloc());
                Tools::Destroy(p,__ref_alloc());
            }
            p = 0;
        }
    }
private:
    _ref_imp(const _ref_imp &);
    _ref_imp & operator =(const _ref_imp &);
public:
    Elem *  ptr_;
private:
    IntType cnt_;
};

NS_IMPL_END

#endif

#ifndef DOZERG_COMMAND_IMPL_H_20081018
#define DOZERG_COMMAND_IMPL_H_20081018

#include <common/Tools.h>

NS_IMPL_BEGIN

template<class T>
void deleteCmd(T * p)
{
    Tools::Destroy(p,__DZ_ALLOC<T>());
}

NS_IMPL_END

//just for convenience use in CreateCommand and ReleaseCommand
#define __CREATE_COMMAND(CMD_CLASS) \
    ret = __DZ_ALLOC<CMD_CLASS>().allocate(1);  \
    DEBUG("create command "<<#CMD_CLASS<<"="<<ret);   \
    new (ret) CMD_CLASS

#define __DELETE_COMMAND(CMD_CLASS) \
    DEBUG("release command "<<#CMD_CLASS<<"=@"<<pCmd<<Tools::ToStringPtr(pCmd));    \
    NS_IMPL::deleteCmd(dynamic_cast<CMD_CLASS *>(pCmd))



#endif

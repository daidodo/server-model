#ifndef DOZERG_COMMAND_IMPL_H_20081018
#define DOZERG_COMMAND_IMPL_H_20081018

#include <common/Tools.h>

NS_IMPL_BEGIN

template<class T>
void deleteCmd(T * p){
    Tools::Destroy(p,__DZ_ALLOC<T>());
}

//just for convenience use in ICommand::CommandName
#define __CMD_CASE(CMD_NAME)  case CMD_NAME:return #CMD_NAME

//just for convenience use in CreateCommand and ReleaseCommand
#define __CREATE_COMMAND(CMD_CLASS) \
    ret = __DZ_ALLOC<CMD_CLASS>().allocate(1);  \
    DEBUG("create command "<<#CMD_CLASS<<"="<<ret);   \
    new (ret) CMD_CLASS

#define __DELETE_COMMAND(CMD_CLASS) \
    DEBUG("release command "<<#CMD_CLASS<<"=@"<<pCmd<<Tools::ToStringPtr(pCmd));    \
    NS_IMPL::deleteCmd(dynamic_cast<CMD_CLASS *>(pCmd))

#define __CREATE_CMD_CASE(CMD_ID,CMD_CLASS) case CMD_ID:{__CREATE_COMMAND(CMD_CLASS);break;}
#define __DELETE_CMD_CASE(CMD_ID,CMD_CLASS) case CMD_ID:{__DELETE_COMMAND(CMD_CLASS);break;}

NS_IMPL_END

#endif

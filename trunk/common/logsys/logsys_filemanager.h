#ifndef DZ_LOGSYS_FILE_MANAGER_20071023
#define DZ_LOGSYS_FILE_MANAGER_20071023

#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <common/impl/Alloc.h>
#include "logsys_config_items.h"

IMPL_BEGIN

class CFileManager
{
    typedef __DZ_VECTOR(__DZ_STRING) __FileVec;
    __DZ_STRING filename_;      //文件名
    size_t      filesz_;        //文件最大size
    __FileVec   backfile_;      //备份文件名
public:
    size_t  FileSize() const{return filesz_;}
    void Config(const CConfigItems & item){
        filename_ = item.filename_;
        filesz_ = item.filesz_;
        backfile_.resize(item.backIndex_,filename_ + ".");
        for(size_t i = 0;i < backfile_.size();++i){
            __DZ_OSTRINGSTREAM oss;
            oss<<(i + 1);
            backfile_[i] += oss.str();
        }
    }
    void RollFile() const{
        if(backfile_.empty()){
            remove(filename_.c_str());
        }else{
            for(size_t i = backfile_.size() - 1;i > 0;--i)
                rename(backfile_[i - 1].c_str(),backfile_[i].c_str());
            rename(filename_.c_str(),backfile_[0].c_str());
        }
    }
    int OpenLogFile() const{
        const int FILE_MODE = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        const int OPEN_FLAG = O_WRONLY | O_APPEND | O_CREAT;
        return open(filename_.c_str(),OPEN_FLAG,FILE_MODE);
    }
};

IMPL_END

#endif

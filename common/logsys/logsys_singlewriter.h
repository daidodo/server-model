#ifndef DZ_LOGSYS_SIGNLE_WRITER_20071020
#define DZ_LOGSYS_SIGNLE_WRITER_20071020

#include <Configuration.h>
#include "logsys_mutexwriter.h"

IMPL_BEGIN

//保证只有单个writer对象
//选择适当和高效的singleton模式
class CSingleWriter : public CMutexWriter
{
    //日期时间的格式可参考strftime
    std::string time_format_;
    int         level_;
    ~CSingleWriter(){}
    CSingleWriter(){}
    CSingleWriter(const CSingleWriter &);
    CSingleWriter & operator =(const CSingleWriter &);
public:
    static CSingleWriter & Instance(){  //Meyers' singleton
        static CSingleWriter inst;
        return inst;
    }
    void Config(const char * conf){ //conf为配置文件名,为0时采用默认配置
        CConfigItems item;
        if(conf){
            NS_SERVER::CConfiguration config;
            if(!config.Load(conf)){
                assert(0 && "cannot open logsys config file");
            }
            item.level_ = config.GetString("logsys.Lever",item.level_);
            item.filename_ = config.GetString("logsys.Filename",item.filename_);
            item.filesz_ = config.GetInt("logsys.MaxFileSz",item.filesz_,4 * 1024);
            item.backIndex_ = config.GetInt("logsys.MaxBackupIndex",item.backIndex_,0);
            item.timeFormat_ = config.GetString("logsys.TimeFormat",item.timeFormat_);
            item.buffersz_ = config.GetInt("logsys.BufferSize",item.buffersz_,512);
            item.quickFlush_ = config.GetInt("logsys.QuickFlush",item.quickFlush_);
        }
        time_format_ = item.timeFormat_;
        if(item.level_ == "TRACE"){
            level_ = 0;
        }else if(item.level_ == "DEBUG"){
            level_ = 1;
        }else if(item.level_ == "INFO"){
            level_ = 2;
        }else if(item.level_ == "WARN"){
            level_ = 3;
        }else if(item.level_ == "ERROR"){
            level_ = 4;
        }else if(item.level_ == "FATAL"){
            level_ = 5;
        }else if(item.level_ == "OFF"){
            level_ = 6;
        }else{
            assert(0 && "unknown log level");
        }
        CMutexWriter::configMWriter(item);
    }
    const char * TimeFormat() const{return time_format_.c_str();}
    int Level() const{return level_;}
};

IMPL_END

#endif

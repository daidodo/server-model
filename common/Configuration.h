#ifndef DOZERG_CONFIGURATION_H_20070821
#define DOZERG_CONFIGURATION_H_20070821

/*
    配置文件读取
        CConfiguration
    History
        20080131    MAX_LINE_LEN从256改为512
        20080219    去掉MAX_LINE_LEN,即每行字符数限制
        20080614    修正GetInt()，返回ndefault时检查是否在[min,max]范围内
        20081014    增加ToString()函数，输出所有配置项
        20081018    把实现代码转移到.h文件里，去掉Configuration.cpp文件
//*/

#include <map>
#include <limits>           //std::numeric_limits
#include <string>
#include <fstream>          //std::ifstream
#include <cstdlib>          //atoi
#include <common/Tools.h>   //Tools::Trim

/*
配置文件格式：
[SPACE] # comments [ENDLINE]
[SPACE] expression [SPACE] = [SPACE] results [SPACE] [# comments] [ENDLINE]
[SPACE] expression [SPACE] = [SPACE] [# comments] [ENDLINE]
[SPACE] expression [SPACE] results [SPACE] [# comments]
[SPACE] expression [SPACE] [# comments] [ENDLINE]
//*/

NS_SERVER_BEGIN

class CConfiguration
{
    typedef __DZ_MAP(__DZ_STRING,__DZ_STRING)   container_type;
    __DZ_STRING     conf_file_;
    container_type  content_;
public:
    //得到配置文件名
    __DZ_STRING GetFileName() const{return conf_file_;}
    //清除读入的配置项
    void Clear(){content_.clear();}
    //从文件中读取配置项
    bool Load(const char * file_name){
        if(!file_name)
            return false;
        conf_file_ = file_name;
        std::ifstream inf(file_name);
        if(!inf.is_open())
            return false;
        Clear();
        __DZ_STRING line;
        while(!inf.eof()){
            std::getline(inf,line);
            line = line.substr(0,line.find_first_of("#"));
            if(line.empty())
                continue;
            __DZ_STRING::size_type i = line.find_first_of("=");
            if(i == __DZ_STRING::npos){
                line = Tools::Trim(line);
                i = line.find_first_of(" ");
            }
            content_[Tools::Trim(line.substr(0,i))] = Tools::Trim(line.substr(i + 1));
        }
        return true;
    }
    //得到配置项的字符串值
    __DZ_STRING GetString(__DZ_STRING key,__DZ_STRING strdefault = "") const{
        container_type::const_iterator wh = content_.find(key);
        if(wh == content_.end())
            return strdefault;
        return wh->second;
    }
    //得到配置项的整数值
    int GetInt(__DZ_STRING key,int ndefault = 0,
        int min = std::numeric_limits<int>::min(),
        int max = std::numeric_limits<int>::max()) const
    {
        container_type::const_iterator wh = content_.find(key);
        int ret = ndefault;
        if(wh != content_.end()){
            ret = atoi(wh->second.c_str());
            switch(*wh->second.rbegin()){
                case 'k':case 'K':
                    ret <<= 10;
                    break;
                case 'm':case 'M':
                    ret <<= 20;
                    break;
                case 'g':case 'G':
                    ret <<= 30;
                    break;
                default:;
            }
        }
        return (ret < min ? min : (ret > max ? max : ret));
    }
};

NS_SERVER_END

#endif

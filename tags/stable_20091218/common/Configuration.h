#ifndef DOZERG_CONFIGURATION_H_20070821
#define DOZERG_CONFIGURATION_H_20070821

/*
    �����ļ���ȡ
        CConfiguration
    History
        20080131    MAX_LINE_LEN��256��Ϊ512
        20080219    ȥ��MAX_LINE_LEN,��ÿ���ַ�������
        20080614    ����GetInt()������ndefaultʱ����Ƿ���[min,max]��Χ��
        20081014    ����ToString()�������������������
        20081018    ��ʵ�ִ���ת�Ƶ�.h�ļ��ȥ��Configuration.cpp�ļ�
//*/

#include <map>
#include <limits>           //std::numeric_limits
#include <string>
#include <fstream>          //std::ifstream
#include <cstdlib>          //atoi
#include <common/Tools.h>   //Tools::Trim

/*
�����ļ���ʽ��
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
    //�õ������ļ���
    __DZ_STRING GetFileName() const{return conf_file_;}
    //��������������
    void Clear(){content_.clear();}
    //���ļ��ж�ȡ������
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
    //�õ���������ַ���ֵ
    __DZ_STRING GetString(__DZ_STRING key,__DZ_STRING strdefault = "") const{
        container_type::const_iterator wh = content_.find(key);
        if(wh == content_.end())
            return strdefault;
        return wh->second;
    }
    //�õ������������ֵ
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

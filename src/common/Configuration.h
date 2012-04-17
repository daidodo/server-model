#ifndef DOZERG_CONFIGURATION_H_20070821
#define DOZERG_CONFIGURATION_H_20070821

/*
    �����ļ���ȡ
        CConfiguration
    History
        20080131    MAX_LINE_LEN��256��Ϊ512
        20080219    ȥ��MAX_LINE_LEN, ��ÿ���ַ�������
        20080614    ����GetInt()������ndefaultʱ����Ƿ���[min, max]��Χ��
        20081014    ����ToString()�������������������
        20081018    ��ʵ�ִ���ת�Ƶ�.h�ļ��ȥ��Configuration.cpp�ļ�
//*/

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <limits>           //std::numeric_limits
#include <cstdlib>          //atoi
#include <Tools.h>          //Tools::Trim

/*
�����ļ���ʽ��
---- FORMAT_EQUAL ----
[SPACE] # comments [ENDLINE]
[SPACE] expression [SPACE] = [SPACE] results [SPACE] [# comments] [ENDLINE]
[SPACE] expression [SPACE] = [SPACE] [# comments] [ENDLINE]
[SPACE] expression [SPACE] [# comments] [ENDLINE]
---- FORMAT_SPACE ----
[SPACE] # comments [ENDLINE]
[SPACE] expression [SPACE] results [SPACE] [# comments] [ENDLINE]
[SPACE] expression [SPACE] [# comments] [ENDLINE]
//*/

NS_SERVER_BEGIN

class CConfiguration
{
    typedef std::map<std::string, std::string>   container_type;
public:
    static const int FORMAT_EQUAL = 0;
    static const int FORMAT_SPACE = 1;
    //�õ������ļ���
    std::string GetConfFname() const{return conf_file_;}
    //��������������
    void Clear(){content_.clear();}
    //���ļ��ж�ȡ������
    bool Load(const std::string & file_name, int format = FORMAT_EQUAL){
        std::string abs_file = Tools::AbsFilename(file_name);
        std::ifstream inf(abs_file.c_str());
        if(!inf.is_open())
            return false;
        Clear();
        for(std::string line;!inf.eof();){
            std::getline(inf, line);
            parseFormat(line.substr(0, line.find_first_of("#")), format);
        }
        conf_file_ = abs_file;
        return true;
    }
    //�õ���������ַ���ֵ
    std::string GetString(const std::string & key, const std::string & strdefault = "") const{
        container_type::const_iterator wh = content_.find(key);
        if(wh == content_.end())
            return strdefault;
        return wh->second;
    }
    //�õ������������ֵ
    int GetInt(const std::string & key, int ndefault = 0,
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
    //�õ���������ַ���ֵ�������Ҫ��ת�����ļ�����·��
    std::string GetFilepath(const std::string & key, const std::string & strdefault = "") const{
        return Tools::AbsFilename(GetString(key, strdefault));
    }
private:
    void parseFormat(const std::string & line, int format){
        if(line.empty())
            return;
        switch(format){
            case FORMAT_EQUAL:{
                std::string::size_type i = line.find_first_of("=");
                content_[Tools::Trim(line.substr(0, i))] = Tools::Trim((std::string::npos == i ? "" : line.substr(i + 1)));
                break;}
            case FORMAT_SPACE:{
                std::istringstream iss(line);
                std::string key, val;
                iss>>key>>val;
                if(!key.empty())
                    content_[key] = val;
                break;}
            default:;
        }
    }
    std::string     conf_file_;
    container_type  content_;
};

NS_SERVER_END

#endif

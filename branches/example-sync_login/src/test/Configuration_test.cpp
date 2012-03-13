#include "comm.h"

#include <Configuration.h>

static inline std::string keyName(const char * key, int i)
{
    std::ostringstream oss;
    oss<<key<<i;
    return oss.str();
}

int main()
{
    CConfiguration config;
    config.Load("test.conf");
    int valCount = config.GetInt("val.count");
    int emptyCount = config.GetInt("empty.count");
    //test val
    for(int i = 1;i < valCount;++i){
        const std::string key = keyName("val", i);
        const std::string VAL = keyName("VAL", i);
        const std::string val = config.GetString(key);
        if(VAL != val){
            cerr<<"CConfiguration::GetString('"<<key<<"') returns '"<<val<<"' is not '"<<VAL<<"'\n";
            return 1;
        }
    }
    //test empty
    for(int i = 1;i < emptyCount;++i){
        const std::string key = keyName("empty", i);
        const std::string VAL = "";
        const std::string val = config.GetString(key);
        if(VAL != val){
            cerr<<"CConfiguration::GetString('"<<key<<"') returns '"<<val<<"' is not '"<<VAL<<"'\n";
            return 1;
        }
    }
    int empty_1 = config.GetInt("");
    if(1 != empty_1){
        cerr<<"CConfiguration::GetInt('') returns "<<empty_1<<" is not 1\n";
        return -1;
    }
    cout<<"Configuration test succ\n";
    return 0;
}

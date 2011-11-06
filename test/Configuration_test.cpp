#include <common/Configuration.h>

#include "comm.h"

static inline __DZ_STRING keyName(const char * key, int i)
{
    __DZ_OSTRINGSTREAM oss;
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
        const __DZ_STRING key = keyName("val", i);
        const __DZ_STRING VAL = keyName("VAL", i);
        const __DZ_STRING val = config.GetString(key);
        if(VAL != val){
            cerr<<"CConfiguration::GetString('"<<key<<"') returns '"<<val<<"' is not '"<<VAL<<"'\n";
            return 1;
        }
    }
    //test empty
    for(int i = 1;i < emptyCount;++i){
        const __DZ_STRING key = keyName("empty", i);
        const __DZ_STRING VAL = "";
        const __DZ_STRING val = config.GetString(key);
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

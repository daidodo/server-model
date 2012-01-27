#include <sstream>

#include "Events.h"

NS_SERVER_BEGIN

namespace Events{

    std::string ToString(__Events ev)
    {
        const char * const BIT_NAME[EVENT_COUNT] = {
            "CLOSE",
            "IN",
            "OUT",
            "ACCEPT",
            "TCP_RECV",
            "UDP_RECV",
            "TCP_SEND",
            "UDP_SEND",
            "READ",
            "WRITE",
        };
        std::ostringstream oss;
        oss<<ev;
        bool empty = true;
        for(size_t i = 0;i < EVENT_COUNT;++i){
            if(0 == (ev & (1 << i)))
                continue;
            if(empty){
                oss<<"(";
                empty = false;
            }else
                oss<<" | ";
            if(BIT_NAME[i])
                oss<<BIT_NAME[i];
            else
                oss<<std::hex<<"0x"<<(1 << i);
        }
        if(!empty)
            oss<<")";
        return oss.str();
    }

}//namespace Events

NS_SERVER_END

#include <CharBuffer.h>

#include "comm.h"

#define __CharP  (Char *)

template<typename Char>
static bool test()
{
    typedef CCharBuffer<Char> __Buffer;
    Char str[100] = {'a','b', 'c', 'd', 'e', 'f', 'g'};
    //ctor
    __Buffer buf1;
    __Buffer buf2(__CharP"abcdefg");
    const __Buffer buf3(str, sizeof str, 7);
    __Buffer buf4 = __CharP"abcdefg";
    if(buf1 == buf2)
        return false;
    if(buf2 != buf3)
        return false;
    if(buf3 != buf4)
        return false;
    if(buf2 != buf4)
        return false;
    if(!buf1.empty())
        return false;
    if(7 != buf2.size() || 7 != buf2.capacity())
        return false;
    if(sizeof str != buf3.capacity())
        return false;
    if(7 != buf4.capacity())
        return false;
    if(7 != buf4.length())
        return false;
    //assign
    buf1.assign(buf3);
    if(buf1 != buf3)
        return false;
    if(sizeof str != buf1.capacity())
        return false;
    buf2.assign(str, sizeof str, 7);
    if(buf2 != buf3)
        return false;
    if(sizeof str != buf2.capacity())
        return false;
    //copy
    Char str2[8] = {};
    if(7 != buf2.copy(str2, 7))
        return false;
    if(0 != strcmp((const char *)str, (const char *)str2))
        return false;
    //get_allocator
    typename __Buffer::allocator_type a = buf1.get_allocator();
    //begin, end
    typename __Buffer::value_type ch = 'A';
    typedef typename __Buffer::iterator __Iter;
    typedef typename __Buffer::const_iterator __CIter;
    for(__Iter i = buf1.begin();i != buf1.end();++i)
        *i = ch++;
    ch = 'A';
    for(__CIter i = buf3.begin();i != buf3.end();++i)
        if(*i != ch++)
            return false;
    //rbegin, rend
    typedef typename __Buffer::reverse_iterator __RIter;
    typedef typename __Buffer::const_reverse_iterator __CRIter;
    ch = 'A';
    for(__RIter i = buf1.rbegin();i != buf1.rend();++i)
        *i = ch++;
    ch = 'A';
    for(__CRIter i = buf3.rbegin();i != buf3.rend();++i)
        if(*i != ch++)
            return false;
    //operator []
    ch = 'A';
    for(size_t i = 0;i < buf1.size();++i)
        buf1[i] = ch++;
    ch = 'A';
    for(size_t i = 0;i < buf3.size();++i)
        if(buf3[i] != ch++)
            return false;
    //back
    buf1.back() = 'x';
    if(buf3.back() != 'x')
        return false;
    //at
    ch = 'A';
    for(size_t i = 0;i < buf1.size();++i)
        buf1.at(i) = ch++;
    ch = 'A';
    for(size_t i = 0;i < buf3.size();++i)
        if(buf3.at(i) != ch++)
            return false;
    //clear, empty
    if(buf2.empty())
        return false;
    buf2.clear();
    if(!buf2.empty())
        return false;
    //resize
    buf2.resize(7);
    if(7 != buf2.size())
        return false;
    //push_back
    buf2.clear();
    if(buf2 == buf4)
        return false;
    ch = 'a';
    for(int i = 0;i < 7;++i)
        buf2.push_back(ch + i);
    if(buf2 != buf4)
        return false;
    //swap
    buf2.swap(buf4);
    if(str == &buf2[0])
        return false;
    if(str != &buf4[0])
        return false;
    swap(buf4, buf2);
    if(str == &buf4[0])
        return false;
    if(str != &buf2[0])
        return false;
    //append
    buf1.clear();
    for(size_t i = 0;i < 10;++i)
        buf1.append(10, 'd');
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();++i)
        if(buf1[i] != 'd')
            return false;
    buf1.clear();
    for(size_t i = 0;i < 20;++i)
        buf1.append(__CharP"ssdeb");
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();i += 5)
        if(0 != memcmp(&buf1[i], "ssdeb", 5))
            return false;
    buf4.assign(__CharP"gadfdengb");
    buf1.clear();
    for(size_t i = 0;i < 20;++i)
        buf1.append(buf4, 4, 5);
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();i += 5)
        if(0 != memcmp(&buf1[i], "dengb", 5))
            return false;
    buf4.resize(5);
    buf1.clear();
    for(size_t i = 0;i < 20;++i)
        buf1.append(buf4);
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();i += 5)
        if(0 != memcmp(&buf1[i], "gadfd", 5))
            return false;
    //operator +=
    buf1.clear();
    for(size_t i = 0;i < 20;++i)
        buf1 += __CharP"sgdeb";
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();i += 5)
        if(0 != memcmp(&buf1[i], "sgdeb", 5))
            return false;
    buf4.assign(__CharP"f90rg");
    buf1.clear();
    for(size_t i = 0;i < 20;++i)
        buf1 += buf4;
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();i += 5)
        if(0 != memcmp(&buf1[i], "f90rg", 5))
            return false;
    buf1.clear();
    for(int i = 0;i < 100;++i)
        buf1 += 12 + i;
    if(100 != buf1.size())
        return false;
    for(size_t i = 0;i < buf1.size();++i)
        if(buf1[i] != 12 + int(i)){
            cerr<<"buf1["<<i<<"]="<<int(buf1[i])<<" is not "<<(12 + int(i))<<endl;
            return false;
        }
    //insert
    buf4.assign(__CharP"3e4r5");
    buf1.clear();
    buf1 += buf4;
    if(5 != buf1.size())
        return false;
    for(int i = 0;i < 19;++i)
        buf1.insert(i * 5 + 3, 5, 'a');
    if(100 != buf1.size()){
        cerr<<"1: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"1: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 3;i < 98;++i)
        if('a' != buf1[i])
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    buf1.clear();
    buf1 += buf4;
    for(int i = 0;i < 19;++i)
        buf1.insert(i * 5 + 3, __CharP"3g89a", 5);
    if(100 != buf1.size()){
        cerr<<"2: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"2: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 0;i < 19;++i)
        if(0 != memcmp("3g89a", &buf1[i * 5 + 3], 5))
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    buf1.clear();
    buf1 += buf4;
    for(int i = 0;i < 19;++i)
        buf1.insert(i * 5 + 3, __CharP"3g89a");
    if(100 != buf1.size()){
        cerr<<"3: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"3: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 0;i < 19;++i)
        if(0 != memcmp("3g89a", &buf1[i * 5 + 3], 5))
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    buf4.assign(__CharP"23e32t2g22");
    buf1.clear();
    buf1.append(buf4, 0, 5);
    for(int i = 0;i < 19;++i)
        buf1.insert(i * 5 + 3, buf4, 4, 5);
    if(100 != buf1.size()){
        cerr<<"4: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"4: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 0;i < 19;++i)
        if(0 != memcmp(&buf4[4], &buf1[i * 5 + 3], 5))
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    buf4.resize(5);
    buf1.clear();
    buf1.append(buf4);
    for(int i = 0;i < 19;++i)
        buf1.insert(i * 5 + 3, buf4);
    if(100 != buf1.size()){
        cerr<<"5: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"5: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 0;i < 19;++i)
        if(0 != memcmp(&buf4[0], &buf1[i * 5 + 3], 5))
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    buf1.clear();
    buf1.append(buf4);
    for(int i = 0;i < 95;++i){
        __Iter p = buf1.insert(buf1.begin() + i + 3, 12 + i);
        if(*p != 12 + i)
            return false;
    }
    if(100 != buf1.size()){
        cerr<<"6: buf1.size()="<<buf1.size()<<" is not 100\n";
        return false;
    }
    if(0 != memcmp(&buf1[0], &buf4[0], 3)){
        cerr<<"6: buf1[0]="<<&buf1[0]<<", buf4[0]="<<&buf4[0]<<endl;
        return false;
    }
    for(int i = 0;i < 95;++i)
        if(12 + i != buf1[i + 3])
            return false;
    if(0 != memcmp(&buf1[98], &buf4[3], 2))
        return false;
    //replace
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, 3, '0');
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, 5, '1');
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 5, 3, 'a');
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, __CharP"000", 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, __CharP"11111", 5);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 5, __CharP"aaa", 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, __CharP"000");
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, __CharP"11111");
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 5, __CharP"aaa");
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"00011111aaa");
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, buf4, 0, 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 3, buf4, 3, 5);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(4, 5, buf4, 8, 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"000");
    buf1.replace(4, 3, buf4);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"11111");
    buf1.replace(4, 3, buf4);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"aaa");
    buf1.replace(4, 5, buf4);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;

    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, 3, '0');
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, 5, '1');
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 9, 3, 'a');
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, __CharP"000", 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, __CharP"11111", 5);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 9, __CharP"aaa", 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, __CharP"000");
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, __CharP"11111");
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 9, __CharP"aaa");
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"00011111aaa");
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, buf4, 0, 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, buf4, 3, 5);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf1.replace(buf1.begin() + 4, buf1.begin() + 9, buf4, 8, 3);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"000");
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, buf4);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123000789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"11111");
    buf1.replace(buf1.begin() + 4, buf1.begin() + 7, buf4);
    if(12 != buf1.size())
        return false;
    if(0 != memcmp("012311111789", &buf1[0], buf1.size()))
        return false;
    buf4.assign(__CharP"aaa");
    buf1.replace(buf1.begin() + 4, buf1.begin() + 9, buf4);
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123aaa789", &buf1[0], buf1.size()))
        return false;
    //erase
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    __Iter p = buf1.erase(buf1.begin() + 1, buf1.end() - 2);
    if(*p != '8')
        return false;
    if(3 != buf1.size())
        return false;
    if(0 != memcmp("089", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    p = buf1.erase(buf1.begin() + 5);
    if(*p != '6')
        return false;
    if(9 != buf1.size())
        return false;
    if(0 != memcmp("012346789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.erase(3, 4);
    if(6 != buf1.size())
        return false;
    if(0 != memcmp("012789", &buf1[0], buf1.size()))
        return false;
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf1.erase(6);
    if(6 != buf1.size()){
        cerr<<"buf1.size()="<<buf1.size()<<" is not 6\n";
        return false;
    }
    return true;
    if(0 != memcmp("012345", &buf1[0], buf1.size()))
        return false;
    //substr
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(0 != memcmp("0123456789", &buf1[0], buf1.size()))
        return false;
    buf4 = buf1.substr(2, 7);
    if(7 != buf4.size())
        return false;
    if(0 != memcmp("2345678", &buf4[0], buf4.size()))
        return false;
    //compare
    buf1.clear();
    buf1 += __CharP"0123456789";
    if(10 != buf1.size())
        return false;
    if(buf1.compare(2, 5, __CharP"256444", 5))
        return false;
    if(!buf1.compare(2, 5, __CharP"23456444", 5))
        return false;
    if(buf1.compare(2, 5, __CharP"256444"))
        return false;
    if(!buf1.compare(2, 5, __CharP"23456"))
        return false;
    if(buf1.compare(__CharP"23425"))
        return false;
    if(!buf1.compare(__CharP"0123456"))
        return false;
    buf4.assign(__CharP"238623456444");
    if(buf1.compare(2, 6, buf4, 4, 8))
        return false;
    if(!buf1.compare(2, 5, buf4, 4, 8))
        return false;
    buf4.assign(__CharP"2345626234");
    if(buf1.compare(2, 6, buf4))
        return false;
    if(!buf1.compare(2, 5, buf4))
        return false;
    buf4.assign(__CharP"2623424");
    if(buf1.compare(buf4))
        return false;
    if(buf1 == buf4)
        return false;
    buf4.assign(__CharP"0123456789");
    if(!buf1.compare(buf4))
        return false;
    if(buf1 != buf4)
        return false;
    //operators ==, !=, <, <=, >, >=
    buf4.assign(__CharP"0123455789ag");
    if((buf1 == buf4))
        return false;
    if(!(buf1 != buf4))
        return false;
    if((buf1 < buf4))
        return false;
    if((buf1 <= buf4))
        return false;
    if(!(buf1 > buf4))
        return false;
    if(!(buf1 >= buf4))
        return false;
    buf4.assign(__CharP"012345678");
    if((buf1 == buf4))
        return false;
    if(!(buf1 != buf4))
        return false;
    if((buf1 < buf4))
        return false;
    if((buf1 <= buf4))
        return false;
    if(!(buf1 > buf4))
        return false;
    if(!(buf1 >= buf4))
        return false;
    buf4.assign(__CharP"0123456789");
    if(!(buf1 == buf4))
        return false;
    if((buf1 != buf4))
        return false;
    if((buf1 < buf4))
        return false;
    if(!(buf1 <= buf4))
        return false;
    if((buf1 > buf4))
        return false;
    if(!(buf1 >= buf4))
        return false;
    buf4.assign(__CharP"012345688");
    if((buf1 == buf4))
        return false;
    if(!(buf1 != buf4))
        return false;
    if(!(buf1 < buf4))
        return false;
    if(!(buf1 <= buf4))
        return false;
    if((buf1 > buf4))
        return false;
    if((buf1 >= buf4))
        return false;
    buf4.assign(__CharP"01234567890");
    if((buf1 == buf4))
        return false;
    if(!(buf1 != buf4))
        return false;
    if(!(buf1 < buf4))
        return false;
    if(!(buf1 <= buf4))
        return false;
    if((buf1 > buf4))
        return false;
    if((buf1 >= buf4))
        return false;

    const Char * cstr = __CharP"0123455789ag";
    if((buf1 == cstr))
        return false;
    if(!(buf1 != cstr))
        return false;
    if((buf1 < cstr))
        return false;
    if((buf1 <= cstr))
        return false;
    if(!(buf1 > cstr))
        return false;
    if(!(buf1 >= cstr))
        return false;
    cstr = __CharP"012345678";
    if((buf1 == cstr))
        return false;
    if(!(buf1 != cstr))
        return false;
    if((buf1 < cstr))
        return false;
    if((buf1 <= cstr))
        return false;
    if(!(buf1 > cstr))
        return false;
    if(!(buf1 >= cstr))
        return false;
    cstr = __CharP"0123456789";
    if(!(buf1 == cstr))
        return false;
    if((buf1 != cstr))
        return false;
    if((buf1 < cstr))
        return false;
    if(!(buf1 <= cstr))
        return false;
    if((buf1 > cstr))
        return false;
    if(!(buf1 >= cstr))
        return false;
    cstr = __CharP"012345688";
    if((buf1 == cstr))
        return false;
    if(!(buf1 != cstr))
        return false;
    if(!(buf1 < cstr))
        return false;
    if(!(buf1 <= cstr))
        return false;
    if((buf1 > cstr))
        return false;
    if((buf1 >= cstr))
        return false;
    cstr = __CharP"01234567890";
    if((buf1 == cstr))
        return false;
    if(!(buf1 != cstr))
        return false;
    if(!(buf1 < cstr))
        return false;
    if(!(buf1 <= cstr))
        return false;
    if((buf1 > cstr))
        return false;
    if((buf1 >= cstr))
        return false;

    cstr = __CharP"0123455789";
    buf4.assign(__CharP"0123455789ag");
    if((cstr == buf4))
        return false;
    if(!(cstr != buf4))
        return false;
    if((cstr < buf4))
        return false;
    if((cstr <= buf4))
        return false;
    if(!(cstr > buf4))
        return false;
    if(!(cstr >= buf4))
        return false;
    buf4.assign(__CharP"012345678");
    if((cstr == buf4))
        return false;
    if(!(cstr != buf4))
        return false;
    if((cstr < buf4))
        return false;
    if((cstr <= buf4))
        return false;
    if(!(cstr > buf4))
        return false;
    if(!(cstr >= buf4))
        return false;
    buf4.assign(__CharP"0123456789");
    if(!(cstr == buf4))
        return false;
    if((cstr != buf4))
        return false;
    if((cstr < buf4))
        return false;
    if(!(cstr <= buf4))
        return false;
    if((cstr > buf4))
        return false;
    if(!(cstr >= buf4))
        return false;
    buf4.assign(__CharP"012345688");
    if((cstr == buf4))
        return false;
    if(!(cstr != buf4))
        return false;
    if(!(cstr < buf4))
        return false;
    if(!(cstr <= buf4))
        return false;
    if((cstr > buf4))
        return false;
    if((cstr >= buf4))
        return false;
    buf4.assign(__CharP"01234567890");
    if((cstr == buf4))
        return false;
    if(!(cstr != buf4))
        return false;
    if(!(cstr < buf4))
        return false;
    if(!(cstr <= buf4))
        return false;
    if((cstr > buf4))
        return false;
    if((cstr >= buf4))
        return false;

    return true;
}

int main()
{
    if(!test<char>())
        return 1;
    if(!test<signed char>())
        return 1;
    if(!test<unsigned char>())
        return 1;
    cout<<"CharBuff test succ\n";
    return 0;
}

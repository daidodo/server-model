#include <cassert>
#include <string>
#include <vector>
#include <iostream>

#include <common/CompressorLZO.h>
#include <common/CompressorQuickLZ.h>
#include <common/CompressorZLib.h>

using namespace std;
using namespace NS_SERVER;

template<class CompType>
static bool testComp(const char * compName)
{
    assert(compName);
    __DZ_STRING origin("1234567890abcdefghijklmnopeaibanadj;3apU(#JDFNPDdbdn(NNEndjafefJJENFbn apfa3 j[f asdfas]a\\3a0fa sdjfna3 nadka.fj3jadshadsfa");
    CompType comp;
    __DZ_STRING encry;
    int i = comp.Compress(origin, encry);
    if(0 != i){
        cerr<<compName<<"::Compress(string) returns "<<i<<endl;
        return false;
    }
    __DZ_STRING decry;
    i = comp.Decompress(encry, decry);
    if(0 != i){
        cerr<<compName<<"::Decompress(encry="<<Tools::Dump(encry)<<") returns "<<i<<endl;
        return false;
    }
    if(origin != decry){
        cerr<<compName<<": origin="<<Tools::Dump(origin)<<" is diff from decry="<<Tools::Dump(decry)<<endl;
        return false;
    }
    return true;
}

int main()
{
    if(!testComp<CCompressorLZO>("CCompressorLZO"))
        return 1;
    if(!testComp<CCompressorQuickLZ>("CCompressorQuickLZ"))
        return 1;
    if(!testComp<CCompressorZLib>("CCompressorZLib"))
        return 1;
    cout<<"Compressor test succ\n";
    return 0;
}

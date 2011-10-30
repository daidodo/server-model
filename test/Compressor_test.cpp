#include <string>
#include <vector>
#include <iostream>

#include <common/CompressorLZO.h>
#include <common/CompressorQuickLZ.h>

using namespace std;
using namespace NS_SERVER;

int main()
{
    __DZ_STRING origin("1234567890abcdefghijklmnopeaibanadj;3apU(#JDFNPDdbdn(NNEndjafefJJENFbn apfa3 j[f asdfas]a\\3a0fa sdjfna3 nadka.fj3jadshadsfa");
    CCompressorLZO comp;
    __DZ_STRING encry;
    int i = comp.Compress(origin, encry);
    if(0 != i){
        cerr<<"CCompressorLZO::Compress(string) returns "<<i<<endl;
        return 1;
    }
    __DZ_STRING decry;
    i = comp.Decompress(encry, decry);
    if(0 != i){
        cerr<<"CCompressorLZO::Decompress(string) returns "<<i<<endl;
        return 1;
    }
    if(origin != decry){
        cerr<<"origin="<<Tools::Dump(origin)<<" is diff from decry="<<Tools::Dump(decry)<<endl;
        return 1;
    }
    cout<<"CCompressorLZO test succ\n";
    return 0;
}

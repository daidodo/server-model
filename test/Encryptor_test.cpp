#include "comm.h"

#include <common/EncryptorAes.h>

static __DZ_STRING text("1234567890abcdefghijklmnopeaibanadj;3apU(#JDFNPDdbdn(NNEndjafefJJENFbn apfa3 j[f asdfas]a\\3a0fa sdjfna3 nadka.fj3jadshadsfa");

template<class CompType, class BufType>
static bool testCompBuf(const char * compName)
{
    assert(compName);
    CompType comp;
    comp.SetKey("this is key");
    BufType origin(text.begin(), text.end());
    BufType encry;
    int i = comp.Encrypt(origin, encry);
    if(0 != i){
        cerr<<compName<<"::Encrypt(string) returns "<<i<<endl;
        return false;
    }
    BufType decry;
    i = comp.Decrypt(encry, decry);
    if(0 != i){
        cerr<<compName<<"::Decrypt(encry="<<Tools::Dump(&encry[0], encry.size())<<") returns "<<i<<endl;
        return false;
    }
    if(origin != decry){
        cerr<<compName<<": origin="<<Tools::Dump(&origin[0],origin.size())<<" is diff from decry="<<Tools::Dump(&decry[0], decry.size())<<endl;
        return false;
    }
    return true;
}

template<class CompType>
static bool testComp(const char * compName)
{
    if(!testCompBuf<CompType, __DZ_STRING>(compName))
        return false;
    if(!testCompBuf<CompType, __DZ_VECTOR(char)>(compName))
        return false;
    if(!testCompBuf<CompType, __DZ_VECTOR(signed char)>(compName))
        return false;
    if(!testCompBuf<CompType, __DZ_VECTOR(unsigned char)>(compName))
        return false;
    return true;
}

int main()
{
    if(!testComp<CEncryptorAes>("CEncryptorAes"))
        return 1;
    cout<<"Encryptor test succ\n";
    return 0;
}

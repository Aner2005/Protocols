//
// Created by shalevos on 20/06/16.
//

#ifndef NTLWRAPPER_ZZPRECOMPUTEEXP_H
#define NTLWRAPPER_ZZPRECOMPUTEEXP_H

#include <NTL/ZZ.h>


using namespace std;
using namespace NTL;

class ZZPrecomputeExp {
public:
    ZZPrecomputeExp();
    void init(const ZZ &g, const ZZ &N);
    void initDLSE(const ZZ &g, const ZZ &N,const int DLSEconst);
    void prepExpTableMSB();
    ZZ compExp(const ZZ &exp);
    ZZ compInvGen(const ZZ &exp);
    ZZ compExpDLSE(const ZZ &exp);

private:
    ZZ m_g;
    ZZ m_N;
    ZZ *m_exponents;
    int m_DLSEconst;
};


#endif //NTLWRAPPER_ZZPRECOMPUTEEXP_H

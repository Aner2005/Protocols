//
// Created by shalevos on 20/06/16.
//

#ifndef NTLWRAPPER_ZZPRECOMPUTEEXP_SEC_H
#define NTLWRAPPER_ZZPRECOMPUTEEXP_SEC_H

#include <NTL/ZZ.h>

using namespace std;
using namespace NTL;

class ZZPrecomputeExp_Sec  {
public:
    ZZPrecomputeExp_Sec();
    void init(const ZZ &g, const ZZ &N);
    void prepExpTableMSB();
    void prepExpTableSpecifyBitLength(const long size);
    ZZ compExp(const ZZ &exp);


private:
    ZZ m_g;
    ZZ m_N;
    ZZ *m_exponents;
    ZZ m_constant_bit_length;
};


#endif //NTLWRAPPER_ZZPRECOMPUTEEXP_SEC_H

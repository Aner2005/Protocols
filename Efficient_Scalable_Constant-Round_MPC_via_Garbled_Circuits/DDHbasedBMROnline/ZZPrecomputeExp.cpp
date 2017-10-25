//
// Created by shalevos on 20/06/16.
//

#include "ZZPrecomputeExp.h"

ZZPrecomputeExp::ZZPrecomputeExp() { }

void ZZPrecomputeExp::init(const ZZ &g, const ZZ &N) {
    m_g = g;
    m_N = N;
}
void ZZPrecomputeExp::initDLSE(const ZZ &g, const ZZ &N,const int DLSEConst)
{
    m_g = g;
    m_N = N;
    m_DLSEconst=DLSEConst;
}
void ZZPrecomputeExp::prepExpTableMSB() {
    long size = NumBits(m_N);
    long i;
    m_exponents = new ZZ[size];
    ZZ currentG = m_g;

    for (i = 0; i < size; i++) {
        m_exponents[i] = currentG;
        MulMod(currentG,currentG,currentG,m_N);
    }
}

ZZ ZZPrecomputeExp::compExp(const ZZ &exp) {
    ZZ currentExp = exp;
    ZZ result;
    int index = 0;
    result = 1;
    while (currentExp >= 1)
    {
        if ((currentExp % 2) == 1)
            MulMod(result, result, m_exponents[index], m_N);
        index++;
        currentExp /= 2;
    }
    return result;
}

// #ifdef DLSE
ZZ ZZPrecomputeExp::compExpDLSE(const ZZ &exp)
{
    ZZ currentExp = exp;
    ZZ result;
    //currentExp >> m_DLSEconst;
    //int index=m_DLSEconst;
    int index=0;
    result = 1;

    for (int index = 0; index <= m_DLSEconst+1; index++)
    {

        if ((currentExp % 2) == 1)
            MulMod(result, result, m_exponents[index], m_N);

        currentExp   /= 2;//>>=1;//
    }

    return result;
}
// #endif

ZZ ZZPrecomputeExp::compInvGen(const ZZ &exp)
{
     ZZ ans;
     ans=compExpDLSE(exp);
     InvMod(ans,ans,m_N);
     return ans;
}

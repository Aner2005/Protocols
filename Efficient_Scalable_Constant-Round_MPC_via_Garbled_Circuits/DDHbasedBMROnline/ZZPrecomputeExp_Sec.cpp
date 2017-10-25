//
// Created by shalevos on 20/06/16.
//

#include "ZZPrecomputeExp_Sec.h"

ZZPrecomputeExp_Sec::ZZPrecomputeExp_Sec() { }

void ZZPrecomputeExp_Sec::init(const ZZ &g, const ZZ &N) {
    m_g = g;
    m_N = N;
}

void ZZPrecomputeExp_Sec::prepExpTableMSB() {
    long size = NumBits(m_N);
    prepExpTableSpecifyBitLength(size);
}

void ZZPrecomputeExp_Sec::prepExpTableSpecifyBitLength(const long size) {
    m_constant_bit_length = size;
    long i;
    m_exponents = new ZZ[size];
    ZZ currentG = m_g;

    for (i = 0; i < size; i++) {
        m_exponents[i] = currentG;
        MulMod(currentG,currentG,currentG,m_N);
    }
}

ZZ ZZPrecomputeExp_Sec::compExp(const ZZ &exp) {
    ZZ result, dresult;
    result = 1;
    dresult = 1;

    for (int i = 0; i < m_constant_bit_length; ++i) {
        if (bit(exp,i))
            MulMod(result, result, m_exponents[i], m_N);
        else
            MulMod(dresult, result, m_exponents[i], m_N);
    }
    return result;
}

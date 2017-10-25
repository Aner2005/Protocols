//
// Created by shalevos on 20/06/16.
//

#include "MPZPrecomputeExp_Sec.h"

MPZPrecomputeExp_Sec::MPZPrecomputeExp_Sec() {
    mpz_init(m_g);
    mpz_init(m_N);
}

void MPZPrecomputeExp_Sec::init(const mpz_t &g, const mpz_t &N) {
    mpz_set(m_g,g);
    mpz_set(m_N,N);
}

void MPZPrecomputeExp_Sec::prepExpTableMSB() {
    long size = mpz_sizeinbase (m_N, 2);
    prepExpTableSpecifyBitLength(size);
}

void MPZPrecomputeExp_Sec::prepExpTableSpecifyBitLength(const long size) {
    m_constant_bit_length;
    mpz_init_set_ui(m_constant_bit_length,size);
    long i;
    m_exponents = new mpz_t[size];
    for (int j = 0; j < size; ++j)
        mpz_init(m_exponents[j]);

    mpz_t currentG;
    mpz_init(currentG);
    mpz_set(currentG,m_g);

    for (i = 0; i < size; i++) {
        mpz_set(m_exponents[i],currentG);
        mpz_mul(currentG,currentG,currentG);
        mpz_mod(currentG,currentG,m_N);
    }
}


void MPZPrecomputeExp_Sec::compExp(mpz_t &result, const mpz_t &exp) {
    mpz_t dresult;
    mpz_init_set_ui(dresult,1);
    mpz_init_set_ui(result,1);
    unsigned long bitLength = mpz_get_ui(m_constant_bit_length);
    for (int i = 0; i < bitLength; ++i) {
        if(mpz_tstbit(exp,i)) {
            mpz_mul(result, result, m_exponents[i]);
            mpz_mod(result, result, m_N);
        }
        else{
            mpz_mul(dresult, dresult, m_exponents[i]);
            mpz_mod(dresult, dresult, m_N);
        }
    }
}
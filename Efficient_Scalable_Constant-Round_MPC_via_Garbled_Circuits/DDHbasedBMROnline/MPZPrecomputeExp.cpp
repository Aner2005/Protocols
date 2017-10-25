//
// Created by shalevos on 20/06/16.
//

#include "MPZPrecomputeExp.h"

MPZPrecomputeExp::MPZPrecomputeExp() {
    mpz_init(m_g);
    mpz_init(m_N);
}

void MPZPrecomputeExp::init(const mpz_t &g, const mpz_t &N) {
    mpz_set(m_g,g);
    mpz_set(m_N,N);
}

void MPZPrecomputeExp::prepExpTableMSB() {
    long size = mpz_sizeinbase (m_N, 2);
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

void MPZPrecomputeExp::compExp(mpz_t &result, const mpz_t &exp) {
    mpz_t currentExp, tempMod, two;
    mpz_init(tempMod);
    mpz_init(currentExp);
    mpz_init_set_ui(two,2);
    mpz_set(currentExp,exp);

    int index = 0;
    mpz_set_ui(result,1);

    unsigned long l_exp = mpz_get_ui(currentExp);
    while (l_exp >= 1){
        mpz_mod(tempMod,currentExp,two);
        if (mpz_get_ui(tempMod) == 1) {
            mpz_mul(result, result, m_exponents[index]);
            mpz_mod(result, result, m_N);
        }
        index++;
        mpz_fdiv_q(currentExp,currentExp,two);
        l_exp = mpz_get_ui(currentExp);
    }
}








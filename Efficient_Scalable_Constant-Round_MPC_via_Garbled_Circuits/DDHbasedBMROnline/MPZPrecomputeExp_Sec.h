//
// Created by shalevos on 20/06/16.
//

#ifndef NTLWRAPPER_MPZPRECOMPUTEEXP_SEC_H
#define NTLWRAPPER_MPZPRECOMPUTEEXP_SEC_H

#include <gmp.h>
#include <gmpxx.h>

class MPZPrecomputeExp_Sec {
public:
    MPZPrecomputeExp_Sec();
    void init(const mpz_t &g, const mpz_t &N);
    void prepExpTableMSB();
    void prepExpTableSpecifyBitLength(const long size);
    void compExp(mpz_t &result,const mpz_t &exp);

private:
    mpz_t m_g;
    mpz_t m_N;
    mpz_t *m_exponents;
    mpz_t m_constant_bit_length;
};



#endif //NTLWRAPPER_MPZPRECOMPUTEEXP_SEC_H

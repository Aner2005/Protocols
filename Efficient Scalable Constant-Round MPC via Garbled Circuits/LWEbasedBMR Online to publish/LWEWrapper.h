//
// Created by Aner Ben-Efraim in 2016.
//

#ifndef LWEWRAPPER_LWEWRAPPER_H
#define LWEWRAPPER_LWEWRAPPER_H
#include "GaussianSampler.h"
typedef unsigned char byte;

#define SQRM 1025
#define QBY2 525824
#define QBY4 262912
#define QBY4_TIMES3 788736

#define FWD_CONST1 287369
#define FWD_CONST2 269344

#define INVCONST1 240372
#define INVCONST2 1018817
#define INVCONST3 88581
#define SCALING 1049595

#define COEFFICIENT_ALL_ONES 0x1FFFFF//21 bits

using namespace std;



class LWEWrapper {
public:
    //sampler for Gaussian distribution
    GaussianSampler sampler;
    /*
    *Constructor
    */
    LWEWrapper(int numOfParties);
    /*
    * encryption function
    * ciphertext=publicElement*key+error+message
    */
    void encrypt(uint32_t message[M], uint32_t publicElement[M], uint32_t key[M],uint32_t ciphertext[M]);
    /*
    * decryption function
    * message = ciphertext-publicElement*key  (and make positive + discard error)
    * message is partitioned to key and external value
    */
    void decrypt(uint32_t ciphertext[M], uint32_t publicElement[M], uint32_t key[M], uint32_t keyOut[M], bool& externalValue);
    /*
    * summation of encryptions
    * ciphertextSum=ciphertext1+ciphertext2
    */
    void sumEncryptions(uint32_t ciphertext1[M], uint32_t ciphertext2[M], uint32_t ciphertextSum[M]);
    /*
    * summation of 2 keys
    * keySum=key1+key2
    */
    void sumKeys(uint32_t key1[M], uint32_t key2[M], uint32_t keySum[M]);
    /*
    * Create random vectors
    */
    uint32_t** randomElements(int numOfElements);

    void fwd_ntt(uint32_t a[]);
    void inv_ntt(uint32_t a[M]);
    void rearrange(uint32_t a[M]);
    void inline fwd_ntt_r(uint32_t a[]){fwd_ntt(a);rearrange(a);}
    void inv_ntt_r(uint32_t a[M]){inv_ntt(a);rearrange(a);}


    uint32_t mod(uint64_t a);
    void coefficient_add(uint32_t out[M], uint32_t b[M], uint32_t c[M]);
    void coefficient_mul(uint32_t out[M], uint32_t b[], uint32_t c[]);
    void coefficient_mul_add(uint32_t *result, uint32_t *large1, uint32_t *large2,	uint32_t *large3);
    void coefficient_sub(uint32_t result[M], uint32_t b[M], uint32_t c[M]);




    void sym_priv_key_gen(uint32_t s[M]);

    //void make_postive(uint32_t m[M]);
    void error_gen(uint32_t e[M]);
    void pub_key_gen(uint32_t a[M]);
private:
    //q is the modulus
    uint32_t q;
    //M is the dimension
    int dimension;
    //number of parties given in Constructor
    int numOfParties;
    //vector of q/2
    uint32_t constantVec[M];
    //shifts for correcting negative errors
    uint32_t shiftSmall,shiftBig;
};


#endif //LWEWRAPPER_LWEWRAPPER_H

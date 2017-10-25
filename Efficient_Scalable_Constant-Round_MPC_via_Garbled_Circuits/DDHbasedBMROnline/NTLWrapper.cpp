//
// Created by shalev on 19/06/16.
//

#include "NTLWrapper.h"

NTLWrapper::NTLWrapper() {
    time_t t0;
    ZZ t;
    t = (long) time(&t0);
    SetSeed(t);
}

bool NTLWrapper::isPrimitiveRoot(const ZZ &root, const int n)
{
    ZZ* test=new ZZ[n];
    test[0]=root;
    for (int i=1;i<n;i++)
    {
        multiplyZp(test[i],test[i-1],root);
        for (int j=0;j<i;j++)
        {
            if (test[i]==test[j])
            {
                delete[] test;
                return false;
            }
        }
    }
    delete[] test;
    return true;
}

void NTLWrapper::init(const ZZ &p, const ZZ &q, const int n) {
    m_p = p;
    m_q = q;
    m_n = n;

    ZERO=0;
    ONE=1;
    TWO=2;
    inverseZp(INV_TWO,TWO);
}

void NTLWrapper::getGenerators(ZZ *generators, const int n){
    ZZ gen, gen_squared;
    gen = 2;
    if(PowerMod(gen,m_q,m_p) == 1)
        gen = m_p - 2;

    int i = 0;
    ZZ exp;
    exp = 1;
    gen_squared = MulMod(gen,gen,m_p);
    while (i < n && exp < m_q - 1){
        generators[i] = gen;
        MulMod(gen,gen,gen_squared,m_p);
        exp += 2;
        i++;
    }
}

void NTLWrapper::getRandomGenerators(ZZ *generators, const int n)
{

    ZZ gen, gen_squared;
    for (int i = 0; i < n; i++)
    {
        randomZp(gen);
        gen_squared = MulMod(gen,gen,m_p);
        generators[i]=gen_squared;
    }
}

void NTLWrapper::randomZp(ZZ &random){
    RandomBnd(random,m_p);
}

void NTLWrapper::randomKey(ZZ &random){

#ifdef DLSE
    RandomBnd(random,smallExp);
#else
    RandomBnd(random,m_q/2);
#endif

}


void NTLWrapper::powerZp(ZZ &result, const ZZ &b, const ZZ &e){
    PowerMod(result,b,e,m_p);
}

void NTLWrapper::invGen(ZZ& result, const ZZ& b, const ZZ &e){
    PowerMod(result, b, m_p - 1 - e, m_p);
}

void NTLWrapper::multiplyZp(ZZ &result, const ZZ &a, const ZZ &b){
    MulMod(result,a,b,m_p);
}

void NTLWrapper::inverseZp(ZZ &result, const ZZ &a) {
    InvMod(result,a,m_p);
}

void NTLWrapper::sumZp(ZZ &result, const ZZ &a, const ZZ &b){
    AddMod(result,a,b,m_p);
}

void NTLWrapper::subtractZp(ZZ &result, const ZZ &a, const ZZ &b){
    SubMod(result,a,b,m_p);
}

void NTLWrapper::extractRoot(ZZ &root, const ZZ &squared){
    SqrRootMod(root,squared,m_p);
    if (root>m_p/2)
        subtractZp(root,m_p,root);
}

bool NTLWrapper::getLSB(const ZZ &element) {
    return (bool)IsOdd(element);
}

bool NTLWrapper::randomBool(){
    long randB;
    RandomBnd(randB,2);
    return randB;
}

void NTLWrapper::sha256(byte* data, int len, byte* res_hash, int offset)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, len);
    SHA256_Final(hash, &sha256);
    for (size_t i = 0; i < 32; i++) {
        res_hash[i + offset*32] = hash[i];
    }
}

ZZ NTLWrapper::randomOracle(int N){
    int k = (NumBits(m_p)+255)/256;
    byte* hash = new byte[k*32];
    int h_ctr = k*N;
    byte* byteArray = new byte[4] ;
    for (int i = 0; i < k; i++ , h_ctr++) {
        byteArray[0] = (int)((h_ctr & 0xFF000000) >> 24 );
        byteArray[1] = (int)((h_ctr & 0x00FF0000) >> 16 );
        byteArray[2] = (int)((h_ctr & 0x0000FF00) >> 8 );
        byteArray[3] = (int)((h_ctr & 0X000000FF));
        sha256(byteArray, 4 , hash, i);
    }
    delete byteArray, hash;
    return ZZFromBytes(hash, k*32);
}

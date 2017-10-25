//
// Created by shalevos on 19/06/16.
//

#ifndef NTLWRAPPER_NTLWRAPPER_H
#define NTLWRAPPER_NTLWRAPPER_H
// #define ZERO 0
// #define ONE 1
// #define TWO 2


#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <openssl/sha.h>
#include "homDefs.h"

typedef unsigned char byte;

using namespace std;
using namespace NTL;


class NTLWrapper {
public:
    NTLWrapper();
    // ZZ m_p;
    ZZ ZERO,ONE,TWO,INV_TWO;
    bool isPrimitiveRoot(const ZZ &root, const int n);
    void init(const ZZ &p, const ZZ &q, const int n);
    inline ZZ getP(){return m_p;}
    void getGenerators(ZZ *generators, const int n);
    void getRandomGenerators(ZZ *generators, const int n);
    void randomZp(ZZ &random);
    void randomKey(ZZ &random);
    void randomOracle(int num, ZZ &result);
    bool randomBool();
    void powerZp(ZZ &result, const ZZ &b, const ZZ &e);
    void invGen(ZZ& result, const ZZ& b, const ZZ &e);
    void multiplyZp(ZZ &result, const ZZ &a, const ZZ &b);
    void inverseZp(ZZ &result, const ZZ &a);
    void sumZp(ZZ &result, const ZZ &a, const ZZ &b);
    void subtractZp(ZZ &result, const ZZ &a, const ZZ &b);
    void extractRoot(ZZ &root, const ZZ &squared);
    bool getLSB(const ZZ &element);
    void sha256(byte* data, int len, byte* res_hash, int offset);
    ZZ randomOracle(int N);
    ZZ smallExp,exponent;

private:
    ZZ m_p;
    ZZ m_q;
    int m_n;
};


#endif //NTLWRAPPER_NTLWRAPPER_H

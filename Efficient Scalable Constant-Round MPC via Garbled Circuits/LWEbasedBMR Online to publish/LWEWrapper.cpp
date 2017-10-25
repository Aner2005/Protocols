#include "LWEWrapper.h"

uint32_t primrt_table3[8]={1051648, 32832, 53099, 266008, 7197, 322586, 495187, 204436};
uint32_t omega_table3[8] ={32832, 53099, 266008, 7197, 322586, 495187, 204436, 287369};
uint32_t primrt_omega_table3[9]={1051648, 32832, 53099, 266008, 7197, 322586, 495187, 204436, 287369};
uint32_t primrt_inv_omega_table3[8] = {1051648, 1018817, 287674, 345907, 239496, 766169, 880416, 50675};


LWEWrapper::LWEWrapper(int numOfParties)
{
    sampler=GaussianSampler();
    q=MODULUS;
    dimension=M;
    this->numOfParties=numOfParties;
    for (int i=0;i<dimension;i++)
        constantVec[i]=SQRM;
    shiftSmall=SQRM/2;
    shiftBig=shiftSmall*SQRM+shiftSmall;
}

void  LWEWrapper::encrypt(uint32_t message[M], uint32_t publicElement[M], uint32_t key[M], uint32_t ciphertext[M])
{

    uint32_t error[M];
    error_gen(error);

    coefficient_mul_add(error,message,constantVec,error);
    fwd_ntt_r(error);

    coefficient_mul_add(ciphertext,publicElement,key,error);
}

void  LWEWrapper::decrypt(uint32_t ciphertext[M], uint32_t publicElement[M], uint32_t key[M], uint32_t keyOut[M], bool& externalValue)
{

    coefficient_mul(keyOut,publicElement,key);

    coefficient_sub(keyOut,ciphertext,keyOut);

    inv_ntt_r(keyOut);

    for (int i=0;i<dimension;i++)
    {
        keyOut[i]=keyOut[i]+shiftBig;
        if (keyOut[i]>q) keyOut[i]-=q;

        keyOut[i]/=SQRM;
        if (keyOut[i]>=shiftSmall)
            keyOut[i]-=shiftSmall;
        else
            keyOut[i]=q+keyOut[i]-shiftSmall;
    }
    if (keyOut[0]==0) externalValue=0;
     else {externalValue=1;keyOut[0]=0;}

}

void  LWEWrapper::sumEncryptions(uint32_t ciphertext1[M], uint32_t ciphertext2[M], uint32_t ciphertextSum[M])
{
    coefficient_add(ciphertextSum,ciphertext1,ciphertext2);
}

void  LWEWrapper::sumKeys(uint32_t key1[M], uint32_t key2[M], uint32_t keySum[M])
{
    coefficient_add(keySum,key1,key2);
}

uint32_t** LWEWrapper::randomElements(int numOfElements)
{
    uint32_t** ans=new uint32_t*[numOfElements];
    for (int i=0;i<numOfElements;i++)
    {
        ans[i]=new uint32_t[dimension];
        pub_key_gen(ans[i]);
    }
    return ans;
}


//PRIVATE

uint32_t LWEWrapper::mod(uint64_t x)
{
    long long a = (long long) x;
     long long ret2 = (a % MODULUS) >= 0 ? (a % MODULUS) : (a % MODULUS) + MODULUS;
     while (ret2 < 0) {
       ret2 += MODULUS;
     }
     while (ret2 > MODULUS) {
       ret2 -= MODULUS;
     }
   #ifdef DEBUG_PRINTF
     if (!(ret2 >= 0 && ret2 < MODULUS)) {
       printf("error: %d\n", ret2);
     }
   #endif

     if (!(ret2 >= 0 && ret2 < MODULUS))
        cout<<"ERROR: mod incorrect: "<<x<<" -> "<<ret2<<endl;

     return (uint32_t) ret2;
}

void LWEWrapper::coefficient_add(uint32_t out[M], uint32_t b[M], uint32_t c[M])
{
    int j;
    uint32_t temp;
    for (j = 0; j < M; j++)
    {
        out[j]=mod((uint64_t)((uint64_t)b[j] + (uint64_t)c[j]));//maybe remove casting
    }
}

void LWEWrapper::coefficient_mul(uint32_t out[M], uint32_t b[], uint32_t c[])
{
     int j;

     for (j = 0; j < M; j++)
     {
       out[j] = mod((uint64_t)((uint64_t)b[j] * (uint64_t)c[j]));
     }
}

void LWEWrapper::coefficient_mul_add(uint32_t *result, uint32_t *large1, uint32_t *large2,	uint32_t *large3)
{
      int j;
      uint64_t tmp;

      for (j = 0; j < M; j++)
      {
    	tmp = (uint64_t)large1[j] * (uint64_t)large2[j];
        result[j] = mod(tmp + (uint64_t)large3[j]);
      }
}

void LWEWrapper::coefficient_sub(uint32_t result[M], uint32_t b[M], uint32_t c[M])
{
    for (int j = 0; j < M; j++)
    {
      result[j] = mod((uint64_t)((uint64_t)b[j] - (uint64_t)c[j]));
    }
}

void LWEWrapper::fwd_ntt(uint32_t a[])
{
     int i, j, k, m;
     uint64_t u1, t1, u2, t2;
     uint64_t primrt, omega;

     i = 0;
     for (m = 2; m <= M / 2; m = 2 * m) {
       primrt = primrt_omega_table3[i];
       omega = primrt_omega_table3[i + 1];
       i++;

       for (j = 0; j < m; j += 2) {
         for (k = 0; k < M; k = k + 2 * m) {
           u1 = a[j + k];
           t1 = mod(omega * a[j + k + 1]);

           u2 = a[j + k + m];
           t2 = mod(omega * a[j + k + m + 1]);

           a[j + k] = mod(u1 + t1);
           a[j + k + 1] = mod(u2 + t2);

           a[j + k + m] = mod(u1 - t1);
           a[j + k + m + 1] = mod(u2 - t2);

         }
         omega = omega * primrt;
         omega = mod(omega);
       }
     }

     primrt = FWD_CONST1;
     omega = FWD_CONST2;
     for (j = 0; j < M / 2; j++) {
       t1 = omega * a[2 * j + 1];
       t1 = mod(t1);
       u1 = a[2 * j];
       a[2 * j] = mod(u1 + t1);
       a[2 * j + 1] = mod(u1 - t1);

       omega = omega * primrt;
       omega = mod(omega);
     }
}

void LWEWrapper::inv_ntt(uint32_t a[M])
{
     int i, j, k, m;
     uint64_t u1, t1, u2, t2;
     uint64_t primrt, omega;
     primrt = 0;

     i = 0;

     for (m = 2; m <= M / 2; m = 2 * m) {
       primrt = primrt_inv_omega_table3[i];
       i++;

       omega = 1;
       for (j = 0; j < m / 2; j++) {
         for (k = 0; k < M / 2; k = k + m) {
           t1 = omega * a[2 * (k + j) + 1];
           t1 = mod(t1);
           u1 = a[2 * (k + j)];
           t2 = omega * a[2 * (k + j + m / 2) + 1];
           t2 = mod(t2);
           u2 = a[2 * (k + j + m / 2)];

           a[2 * (k + j)] = mod(u1 + t1);
           a[2 * (k + j + m / 2)] = mod(u1 - t1);

           a[2 * (k + j) + 1] = mod(u2 + t2);
           a[2 * (k + j + m / 2) + 1] = mod(u2 - t2);
         }
         omega = omega * primrt;
         omega = mod(omega);
       }
     }

     primrt = INVCONST1;
     omega = 1;
     for (j = 0; j < M;) {
       u1 = a[j];
       j++;
       t1 = omega * a[j];
       t1 = mod(t1);

       a[j - 1] = mod(u1 + t1);
       a[j] = mod(u1 - t1);
       j++;

       omega = omega * primrt;
       omega = mod(omega);
     }
     uint64_t omega2 = INVCONST2;
     primrt = INVCONST3;
     omega = 1;

     for (j = 0; j < M;) {

       a[j] = mod(omega * a[j]);

       a[j] = mod(a[j] * (uint64_t)SCALING);

       j++;
       a[j] = mod(omega2 * a[j]);

       a[j] = mod(a[j] * (uint64_t)SCALING);
       j++;

       omega = omega * primrt;
       omega = mod(omega);
       omega2 = omega2 * primrt;
       omega2 = mod(omega2);
     }
}

void LWEWrapper::rearrange(uint32_t a[M])
{
      #define LOGDIM 8
      uint32_t i,j,k;
      uint32_t bit[LOGDIM];
      uint32_t swp_index;

      uint32_t u1, u2;

      for (i = 1; i < M / 2; i++) {
          bit[0]=i%2;
          for (j=1;j<LOGDIM;j++)
          {
              bit[j]=(i>>j) % 2;
          }

          swp_index=bit[LOGDIM-1];k=1;
          for (j=1;j<LOGDIM;j++)
          {
              k*=2;
              swp_index+=bit[LOGDIM-1-j]*k;
          }


        if (swp_index > i) {
          u1 = a[2 * i];
          u2 = a[2 * i + 1];

          a[2 * i] = a[2 * swp_index];
          a[2 * i + 1] = a[2 * swp_index + 1];

          a[2 * swp_index] = u1;
          a[2 * swp_index + 1] = u2;
        }
      }
}

void LWEWrapper::sym_priv_key_gen(uint32_t s[M])
{
    sampler.knuth_yao2(s);
}

void LWEWrapper::error_gen(uint32_t e[M])
{
    sampler.knuth_yao2(e);
}

void LWEWrapper::pub_key_gen(uint32_t a[M])
{
    //TODO:replace
    for (int i=0;i<M;i++)
        a[i]=rand();
    fwd_ntt_r(a);
}

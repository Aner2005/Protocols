#ifndef GaussianSampler_H
#define GaussianSampler_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define NEW_RND_BOTTOM 1
#define NEW_RND_LARGE 32 - 9
#define NEW_RND_MID 32 - 6

#define LOW_MSB 22
#define HAMMING_TABLE_SIZE 8
#define PMAT_MAX_COL 109
#define KN_DISTANCE1_MASK 7
#define KN_DISTANCE2_MASK 15


#define MODULUS 1051649
#define M 512

class GaussianSampler {
public:
GaussianSampler();

void sample_Gaussian(uint32_t* a, int length);

void knuth_yao2(uint32_t a[M]);

uint32_t knuth_yao_single_number(uint32_t *rnd, int * sample_in_table);

private:
};

#endif//GaussianSampler_H

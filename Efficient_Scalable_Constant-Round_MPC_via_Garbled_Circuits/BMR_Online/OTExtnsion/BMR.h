/*
 * 	BMR.h
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016 & 2017
 *
 */

#ifndef _BMR_H_
#define _BMR_H_


//#include "ottmain.h"
#include "secCompMultiParty.h"
//#include "basicSockets.h"
//#include "BMR_BGW.h"
#include "../util/TedKrovetzAesNiWrapperC.h"



using namespace std;

#define func(a,b)           a+(8-a%8)//a//
#define PAD(a,padTo)		a+(padTo-a%padTo)//a//


Circuit* loadCircuit(char* filename);
//the circuit
extern Circuit* cyc;



void loadInputs(char* filename);

void newLoadRandom();


//
void freeXorCompute();
__m128i* XORsuperseeds(__m128i *superseed1, __m128i *superseed2);
void XORsuperseedsNew(__m128i *superseed1, __m128i *superseed2, __m128i *out);

void computeGates();


bool* computeOutputs();

void deletePartial();
void deleteAll();

//NEW - fake generation
void generateFakeGarbledCircuit(Circuit* c, int partyNumber);
void fakeInputSuperseeds();

#endif _BMR_H_

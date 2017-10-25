/*
 * 	BMR.h
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016
 *
 */

#ifndef _BMR_H_
#define _BMR_H_


#include "secCompMultiParty.h"
#include "basicSockets.h"
#include "BMR_BGW.h"
#include "../util/TedKrovetzAesNiWrapperC.h"



using namespace std;

#define func(a,b)           a+(8-a%8)//a//
#define PAD(a,padTo)		a+(padTo-a%padTo)//a//


Circuit* loadCircuit(char* filename);
void loadInputs(char* filename);
//
void initCommunication(string addr, int port, int player, int mode);
void initializeCommunication(int* ports);
void initializeCommunication(char* filename, Circuit* c, int p);
//
void newLoadRandom();


//
void freeXorCompute();
__m128i* XORsuperseeds(__m128i *superseed1, __m128i *superseed2);
void XORsuperseedsNew(__m128i *superseed1, __m128i *superseed2, __m128i *out);

void computeGates();



void exchangeInputs();

void sendInput(bool* inputs, int player);
void receiveInput(int player);

void exchangeSeeds();

void sendSeed(__m128i* seeds, int player);
void receiveSeed(int player);

bool* computeOutputs();

//
void synchronize();
void sendBit(int player, bool* toSend);
void receiveBit(int player);
//
void deletePartial();
void deleteAll();


//*******************************************************************//

void newReceiveGateLatinCrypt(int party, __m128i* summationVector, int totalLength);

void exchangeSeedsOneEvaluator();

#endif _BMR_H_

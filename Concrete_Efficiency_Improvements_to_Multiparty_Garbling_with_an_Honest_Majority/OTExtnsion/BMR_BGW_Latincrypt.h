/*
 * 	BMR_BGW_Latincrypt.h
 *
 *      Author: Aner Ben-Efraim
 *
 *  code done for submission to LatinCrypt 2017
 *
 * 	year: 2017
 *
 */

#ifndef _BMR_BGW__H_
#define _BMR_BGW_H_
#pragma once

#include "BMR_BGW.h"

/*
GLOBAL VARIABLES USED IN BOTH BMR AND BMR_BGW
*/
//the circuit
extern Circuit* cyc;

//for communication
extern string * addrs;
extern BmrNet ** communicationSenders;
extern BmrNet ** communicationReceivers;
//storage room for exchanging data
extern __m128i** playerSeeds;

//number of Wires
//extern int numOfWires;
extern int numOfInputWires;
extern int numOfOutputWires;

//number of players
extern int numOfParties;
//this player number
extern int partyNum;
//the random offset
extern __m128i R;

extern mutex mtxVal;
extern mutex mtxPrint;


extern __m128i* bigMemoryAllocation;

//NEW
// extern unsigned long rCounter;
// extern AES_KEY_TED aes_key;
extern __m128i* RShare;
extern __m128i* allCoefficients;
extern __m128i* allSecrets;
extern int totalSharesNumber;
extern int numOfSmallDegPolynomials;
extern __m128i* shares;


extern int numOfPartiesLC;//number of parties that need to contribute seed - t+1 parties
extern int numOfPartiesLCNew;//same as above, but t+1 in all versions

extern __m128i* privateKeysSent;
extern __m128i* privateKeysReceived;


void initializePrivateKeys();

void donothingTest();

void computeGatesHME();//V

void exchangeSeedsHME();//V

bool* computeOutputsHME();

//*************************//

void secretShareHME();

void exchangeGatesBGWLatinCryptHME();

void exchangeGatesBGWLatinCryptHMElognRounds();

void mulRBGWwShareConversionHME();

//*************************//

void sendSharesHME(int party, int sizeToSend);
//
void receiveSharesHME(int party, int sizeToReceive);
//
void addLocalSharesHME();
//
void generatePolynomialsLatinCryptHME();
//
void computeSharesLatinCryptHME(int p, __m128i* ans);
//
void computeSharesLatinCryptLocalHME(__m128i* ans);

#endif

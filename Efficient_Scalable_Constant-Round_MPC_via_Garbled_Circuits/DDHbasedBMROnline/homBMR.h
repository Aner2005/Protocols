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



using namespace std;

Circuit* loadCircuitHom(char* filename);

bool* computeOutputsHom();

//NEW - fake generation
void generateFakeGarbledCircuitHom(Circuit* c, int partyNumber);
//void fakeOffline();
void fakeInputSuperseedsHom();


#endif

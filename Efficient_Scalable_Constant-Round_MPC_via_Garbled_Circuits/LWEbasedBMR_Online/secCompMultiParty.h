/*
 * secCompMultiParty.h
 *
 *  Created on: September, 2016
 *		Author: Aner Ben-Efraim, based on a file by froike (Roi Inbar) and Aner Ben-Efraim
 *
 */



#ifndef SECCOMPMULTIPARTY_H_
#define SECCOMPMULTIPARTY_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <iomanip>  //cout
#include <iostream>
#include "aes.h"
#include "TedKrovetzAesNiWrapperC.h"
#include "homDefs.h"

//returns a (pseudo)random 32bit number using AES-NI
uint32_t LoadSeedNew();
//initialize randomness from user key using AES-NI
void initializeRandomnessHom(char* key, int numOfParties);
//returns a (pseudo)random bit using AES-NI, by sampling a bit;
bool LoadBool();

////returns the value of the wire
bool getValueHom(Wire wire);

bool getValueHom(Wire *wire);

bool getTrueValueHom(Wire wire);

bool getTrueValueHom(Wire *wire);



//load the inputs
void loadInputsHom(char* filename,Circuit * cyc, int partyNum);

/*
* sets the fanout and return the number of necessary random elements
*/
int setFanOut(Circuit* circuit);

/*
 * This function gets a path to a file that represents a circuit (format instructions below).
 *
 * Returns a Circuit struct.
 *
 */
Circuit * readCircuitFromFile(char* path);

/*
 * private function that creates gate structs, used in readCircuitFromFile function.
 */
Gate GateCreator(const unsigned int inputBit1, const unsigned int inputBit2, const unsigned int outputBit, const unsigned int TTable, const unsigned int flags, unsigned int number);

/*
 * This function prints the Circuit.
 */
void printCircuit(const Circuit * C);

void removeSpacesAndTabs(char* source);

inline void hex_dump(unsigned char* str, int length)
{
	for (int i = 0; i< length; i++)
	{
		printf("%02x", str[i]);
	}
	printf("\n");
}

/*
* This functions as a destroyer for the circuit struct.
*/
void freeCircuit(Circuit * c);
void freeCircuitPartial(Circuit * c);
void freeWire(Wire w);
void freeGate(Gate gate);
void freePlayer(pPlayer p);


#endif /* SECCOMPMULTIPARTY_H_ */

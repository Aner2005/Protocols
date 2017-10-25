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
#include <iomanip>  //cout
#include <iostream>
#include "homDefs.h"

extern NTLWrapper wrapper;


void encrypt(ZZ &result, int num, ZZ key, ZZ toEncrypt);

void decrypt(ZZ &result, int num, ZZ key, ZZ toDecrypt);


void encryptWprecomputation(ZZ &result, ZZPrecomputeExp* generator, ZZ key, ZZ toEncrypt);

void decryptWprecomputation(ZZ &result, ZZPrecomputeExp* generator, ZZ key, ZZ toDecrypt);


ZZ LoadSeedNew();

void initializeRandomnessHom(char* key, int numOfParties);

bool LoadBool();

bool getValueHom(Wire wire);

bool getValueHom(Wire *wire);

bool getTrueValueHom(Wire wire);

bool getTrueValueHom(Wire *wire);

void loadInputsHom(char* filename,Circuit * cyc, int partyNum);

/*
 * This function gets a path to a file that represents a circuit.
 *
 * Returns a Circuit struct.
 *
 */
Circuit * readCircuitFromFile(char path[]);

/*
 * private function that creates gate structs, used in readCircuitFromFile function.
 */
Gate GateCreator(const unsigned int inputBit1, const unsigned int inputBit2, const unsigned int outputBit, const unsigned int TTable, const unsigned int flags, unsigned int number);

/*
 * This function prints the Circuit.
 */
void printCircuit(const Circuit * C);

void removeSpacesAndTabs(char* source);



Gate ** specialGatesCollector(Gate * GatesArray, const unsigned int arraySize, const unsigned int specialAmount);
Gate ** regularGatesCollector(Gate * GatesArray, const unsigned int arraySize, const unsigned int regularGatesAmount);


/*
*prints an arry of field elements
*/
void print(ZZ* arr, int size);

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
void freeCircle(Circuit * c);
void freeCircuitPartial(Circuit * c);
void freeWire(Wire w);
void freeGate(Gate gate);
void freePlayer(Player p);


#endif /* SECCOMPMULTIPARTY_H_ */

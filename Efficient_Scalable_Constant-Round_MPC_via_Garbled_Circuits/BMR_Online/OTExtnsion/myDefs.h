/*
 * 	myDefs.h
 *
 *      Author: Aner Ben-Efraim & froike (Roi Inbar)
 *
 * 	year: 2016 & 2017
 *
 */



#ifndef MYDEFS_H
#define MYDEFS_H

#pragma once
#include <emmintrin.h>

#define VERSION(hm) printVersion(hm);
#define _aligned_malloc(size,alignment) aligned_alloc(alignment,size)
#define _aligned_free free
//returns a (pseudo)random __m128i number using AES-NI
#define RAND LoadSeedNew()
//returns a (pseudo)random bit using AES-NI
#define RAND_BIT LoadBool()

//the encryption scheme
#define PSEUDO_RANDOM_FUNCTION(seed1, seed2, index, numberOfBlocks, result) fixedKeyPseudoRandomFunctionwPipelining(seed1, seed2, index, numberOfBlocks, result);


/*macros for display options*/
//#define PRINT
//#deine PRINT_STEPS

/*macros for functionality*/
#define RANDOM_COMPUTE 256//Size of buffer for random elements
#define TEST_ROUNDS 1//number of times to repeat, for timing testing. TEST_ROUNDS>11 cancels printing of results. TEST_ROUNDS=1 cancels averaging of times.

//Fixed key is shared by all parties
#define FIXED_KEY_AES "43739841701238781571456410093f43"


#define STRING_BUFFER_SIZE 256
#define true 1
#define false 0
#define flagXor 1
#define flagXnor 2
#define flagNone 0
#define dispXor "Xor"
#define dispXnor "Xnor"
#define dispNone "None"

typedef struct truthTable {
	bool FF;
	bool FT;
	bool TF;
	bool TT;
	bool Y1;
	bool Y2;
	bool Y3;
	bool Y4;
} TruthTable;


typedef struct wire {

  unsigned int number;
	bool lambda;//the lambda
	__m128i seed;//the seed
	bool value;//external value - not real value (real value XORed with lambda)


	bool realLambda;//only for input/output wires
	bool realValue;//only for input/output bits

	//for Phase II
	__m128i *superseed;

	//for construction
	bool negation = false;//is the value negated?

	//BGW
	__m128i lambdaShare;

} Wire;


/*
* This struct represents one party and the array of its input bit serials. (Also used for output bits).
*/
typedef struct player {

	unsigned int playerBitAmount;

	//obsolete. Currently used only in construction.
	unsigned int * playerBitArray;

	//replaced by
	Wire **playerWires;//The input wires of the player (or the output wires of the circuit)
} Player;

/*
* This struct represents one gate with two input wires, one output wire and a Truth table.
*/
typedef struct gate {
	int gateNumber;

	Wire *input1;
	Wire *input2;
	Wire *output;


	TruthTable truthTable;

	//Currently supported gates - (shifted) AND, XOR, XNOR, NOT
	unsigned int flags : 2; //limited for 2 bits
	bool flagNOMUL;//multiplication flag - 1 if no multiplication, 0 if there is multiplication
	bool flagNOT; //not flag - 1 if there is not, 0 if there is no not

	bool mulLambdas; //share of lambdaIn1*lambdaIn2

	//New gate values G[0] is Ag, G[1] is Bg, G[2] is Cg, G[3] is Dg.  Use ^Shift for shifted AND gates.
	__m128i *G[4];

	int sh = 0;//shift


	//BGW
	__m128i mulLambdaShare;//share of multiplication of lambdas



} Gate;

typedef struct circuit {

	Gate * gateArray;//all the gates
	Wire * allWires;//all the wires

	Gate  ** specialGates;//array of pointers to freeXOR gates
	Gate  ** regularGates;//array of pointers to multiplication gates



	//number of gates
	unsigned int amountOfGates;
	unsigned int numOfXORGates;
	unsigned int numOfANDGates;

	//number of players
	unsigned int amountOfPlayers;

	//number of wires
	unsigned int amountOfWire;

	Player * playerArray;//input wires of each player
	Player outputWires;// the output wires of the cycle


	unsigned int numOfOutputWires;
	int numOfInputWires;

} Circuit;

#endif

/*
 * 	myDefs.h
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016
 *
 */



#ifndef MYDEFS_H
#define MYDEFS_H

#include "NTLWrapper.h"
#include "ZZPrecomputeExp.h"
#include "ZZPrecomputeExp_Sec.h"
#include "MPZPrecomputeExp.h"
#include "MPZPrecomputeExp_Sec.h"

#pragma once


//randomness
#define RAND LoadSeedNew()
#define RAND_BIT LoadBool()

/*macros for display options*/
//#define PRINT
//#deine PRINT_STEPS

/*macros for functionality*/
#define PRECOMPUTE //No random oracle
#define KAPPA 1023//number of bits of the safe prime
#define DLSE //Short exponent assumption
#define DLSECONST 170//The length of the **collective** key (should be security + log(#parties))
#define TEST_ROUNDS 1//number of iterations



/*
* define section (pre-compilation values)
* STRING_BUFFER_SIZE will describes the maximum buffer length.
*/
#define PAUSE cout<<"Press any key to continue..."<<endl;getchar();
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


inline bool computeGate(TruthTable table, bool in1, bool in2)
{
	if (!in1 && !in2)
		return table.FF;
	else if (!in1 && in2)
		return table.FT;
	else if (in1 && !in2)
		return table.TF;
	else return table.TT;
}

inline bool computeExtVal(TruthTable table, bool lambdaIn1, bool lambdaIn2, int row)
{
	int g;
	if (row==1)
		lambdaIn2=!lambdaIn2;
	else if (row==2)
		lambdaIn1=!lambdaIn1;
	else if (row==3)
	{
		lambdaIn1=!lambdaIn1;
		lambdaIn2=!lambdaIn2;
	}

	return computeGate(table,lambdaIn1,lambdaIn2);
}

typedef struct wire {

	unsigned int number;
	bool lambda;//the lambda
	ZZ keyZero;//the keys
	ZZ keyOne;//the keys
	bool externalValue;//external value - not real value (real value XORed with lambda)


	bool realLambda;//only for input/output wires
	bool realValue;//only for input/output bits

	//for Phase II
	ZZ superseed;//"superseed" is not the correct term anymore (should be replaced by key)

	//for construction
	bool negation = false;//is the value negated?

	//BGW
	ZZ lambdaShare;

	unsigned int fanout;
	int* usedFanouts;

} Wire;


/*
* This struct represents one party and the array of its inputs bit serials. (Also used for output bits).
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
	ZZ G[4];
	//the corresponding external values
	bool externalValues[4];

	int sh = 0;//shift

	//BGW
	ZZ mulLambdaShare;//share of multiplication of lambdas

#ifdef PRECOMPUTE
	int gateFanOutNum;
#endif





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

#ifdef PRECOMPUTE
	ZZPrecomputeExp** generators;
#endif

} Circuit;

#endif

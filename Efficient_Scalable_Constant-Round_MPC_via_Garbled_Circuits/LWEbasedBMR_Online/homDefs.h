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

#pragma once

//randomness
//#define FIXED_KEY_AES "43739841701238781571456410093f43"
#define RANDOM_COMPUTE 256//Size of buffer for random elements
#define RAND LoadSeedNew()
#define RAND_BIT LoadBool()

/*macros for display options*/
//#define PRINT
//#deine PRINT_STEPS

/*macros for functionality*/
//#define PRECOMPUTE //No random oracle
#define MODULUS 1051649
#define M 512//the dimension
#define TEST_ROUNDS 1//number of iterations

#define NEW_RND_BOTTOM 1
#define NEW_RND_LARGE 32 - 9
#define NEW_RND_MID 32 - 6

#define LOW_MSB 22
#define HAMMING_TABLE_SIZE 8
#define PMAT_MAX_COL 109
#define KN_DISTANCE1_MASK 7
#define KN_DISTANCE2_MASK 15

#define SQRM 1025
#define QBY2 525824
#define QBY4 262912
#define QBY4_TIMES3 788736

#define FWD_CONST1 287369
#define FWD_CONST2 269344

#define INVCONST1 240372
#define INVCONST2 1018817
#define INVCONST3 88581
#define SCALING 1049595

#define COEFFICIENT_ALL_ONES 0x1FFFFF//21 bits

using namespace std;


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
	//cout<<lambdaIn1<<" "<<lambdaIn2<<endl;
	//cin >> g;
	return computeGate(table,lambdaIn1,lambdaIn2);
}

typedef struct wire {

	unsigned int number;
	bool lambda;//the lambda
    uint32_t* keyZero;//the keys
    uint32_t* keyOne;//the keys
	bool externalValue;//external value - not real value (real value XORed with lambda)

	bool realLambda;//only for input/output wires
	bool realValue;//only for input/output bits

	//for Phase II
	uint32_t* superseed;//"superseed" is not the correct term anymore (should be replaced by key)

	//for construction
	bool negation = false;//is the value negated?

	//BGW
	uint32_t lambdaShare;

	unsigned int fanout;
	int* usedFanouts;


} Wire;


/*
* This struct represents one player and the array of his inputs bit serials. (Also used for output bits).
*/
typedef struct pPlayer {

	unsigned int playerBitAmount;

	unsigned int * playerBitArray;

	Wire **playerWires;
} pPlayer;

/*
* This struct represents one gate with two input wires, one output wire and a Truth table.
*/
typedef struct gate {
	int gateNumber;

	Wire *input1;
	Wire *input2;
	Wire *output;


	TruthTable truthTable;


	unsigned int flags : 2;
	bool flagNOMUL;
	bool flagNOT;

	bool mulLambdas;

	uint32_t* G[4];

	bool externalValues[4];

	int sh = 0;

	uint32_t mulLambdaShare;

	int gateFanOutNum;

} Gate;

typedef struct circuit {

	Gate * gateArray;//all the gates
	Wire * allWires;//all the wires

	//number of gates
	unsigned int amountOfGates;

	//number of players
	unsigned int amountOfPlayers;

	//number of wires
	unsigned int amountOfWire;

	pPlayer * playerArray;//input wires of each player
	pPlayer outputWires;// the output wires of the cycle


	unsigned int numOfOutputWires;
	int numOfInputWires;


	uint32_t** publicElements;

} Circuit;

#endif

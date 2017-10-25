/*
 * 	BMR.cpp
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016 & 2017
 *
 */

#include "BMR.h"
#include "secCompMultiParty.h"

Circuit* cyc;

int numOfInputWires;
int numOfOutputWires;

int numOfParties;

int partyNum;

__m128i R;

__m128i* bigMemoryAllocation;

Circuit* loadCircuit(char* filename)
{
	cyc = readCircuitFromFile(filename);
	numOfParties = cyc->amountOfPlayers;
	cyc->numOfInputWires = 0;
	for (int p = 0; p < numOfParties; p++)
	{
		cyc->numOfInputWires += cyc->playerArray[p].playerBitAmount;
	}
	numOfInputWires = cyc->numOfInputWires;
	numOfOutputWires = cyc->outputWires.playerBitAmount;
	return cyc;
}


void newLoadRandom()
{

	R = RAND;

	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			cyc->playerArray[p].playerWires[i]->seed = RAND;
			if (p == partyNum)
			{
				cyc->playerArray[partyNum].playerWires[i]->lambda = RAND_BIT;
				cyc->playerArray[partyNum].playerWires[i]->realLambda = cyc->playerArray[partyNum].playerWires[i]->lambda;
			}
			else
			{
				cyc->playerArray[p].playerWires[i]->lambda = 0;
			}
		}
	}
	for (int g = 0; g < cyc->numOfANDGates; g++)
	{
		cyc->regularGates[g]->output->seed = RAND;
		cyc->regularGates[g]->output->lambda = RAND_BIT;
	}

	freeXorCompute();
}

void freeXorCompute()
{
	for (int g = 0; g < cyc->numOfXORGates; g++)
	{
		cyc->specialGates[g]->output->lambda = cyc->specialGates[g]->input1->lambda ^ cyc->specialGates[g]->input2->lambda;//out=in1^in2

		cyc->specialGates[g]->output->seed = _mm_xor_si128(cyc->specialGates[g]->input1->seed, cyc->specialGates[g]->input2->seed);//out=in1^in2
	}
}

__m128i* XORsuperseeds(__m128i *superseed1, __m128i *superseed2)
{

	void* test=_aligned_malloc(numOfParties*sizeof(__m128i), 16);

	__m128i* ans = static_cast<__m128i *> (test);
	for (int p = 0; p < numOfParties; p++)
	{
		ans[p] = _mm_xor_si128(superseed1[p], superseed2[p]);
	}
	return ans;
}

void XORsuperseedsNew(__m128i *superseed1, __m128i *superseed2, __m128i *out)
{
	for (int p = 0; p < numOfParties; p++)
	{
		out[p] = _mm_xor_si128(superseed1[p], superseed2[p]);
	}
}




bool* computeOutputs()
{

	__m128i* arr = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
	bool* outputs = new bool[cyc->outputWires.playerBitAmount];
	bool valueIn1;
	bool valueIn2;
#ifdef BMR_DEBUG
	int t;
#endif
	for (int g = 0; g < cyc->amountOfGates; g++)
	{


		if (cyc->gateArray[g].flagNOMUL)
		{
			XORsuperseedsNew(cyc->gateArray[g].input1->superseed, cyc->gateArray[g].input2->superseed, cyc->gateArray[g].output->superseed);
		}
		else
		{

			valueIn1 = getValue(cyc->gateArray[g].input1, partyNum);
			valueIn2 = getValue(cyc->gateArray[g].input2, partyNum);

			if (!valueIn1&&!valueIn2)
				{cyc->gateArray[g].output->superseed = cyc->gateArray[g].G[0];}
			else if (!valueIn1&&valueIn2)
				{cyc->gateArray[g].output->superseed = cyc->gateArray[g].G[1];}
			else if (valueIn1&&!valueIn2)
				{cyc->gateArray[g].output->superseed = cyc->gateArray[g].G[2];}
			else if (valueIn1&&valueIn2)
				{cyc->gateArray[g].output->superseed = cyc->gateArray[g].G[3];}

			for (int p = 0; p < numOfParties; p++)
			{
				PSEUDO_RANDOM_FUNCTION(cyc->gateArray[g].input1->superseed[p], cyc->gateArray[g].input2->superseed[p], cyc->gateArray[g].gateNumber, numOfParties, arr);

				XORsuperseedsNew(cyc->gateArray[g].output->superseed, arr, cyc->gateArray[g].output->superseed);
			}

		}
	}
	for (int w = 0; w < cyc->outputWires.playerBitAmount; w++)
	{
		outputs[w] = getTrueValue(cyc->outputWires.playerWires[w], partyNum);
	}
	_aligned_free(arr);
	return outputs;
}

void deletePartial()
{
	_aligned_free(bigMemoryAllocation);
	freeCircuitPartial(cyc);
}


//NEW - fake generation
void generateFakeGarbledCircuit(Circuit* c, int partyNumber)
{
	cyc=c;
	partyNum=partyNumber;
	__m128i* Rvec = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
	for (int p = 0; p < numOfParties; p++)
	  Rvec[p]=RAND;
	R = Rvec[partyNum];
	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			cyc->playerArray[p].playerWires[i]->lambda = RAND_BIT;//0;//
			cyc->playerArray[p].playerWires[i]->superseed=static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
			for (int party=0;party<numOfParties;party++)
			  cyc->playerArray[p].playerWires[i]->superseed[party] = RAND;
			cyc->playerArray[p].playerWires[i]->seed=cyc->playerArray[p].playerWires[i]->superseed[partyNum];


		}
	}

	for (int g = 0; g < cyc->numOfANDGates; g++)
	{
		cyc->regularGates[g]->output->superseed=static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
		cyc->regularGates[g]->output->lambda = RAND_BIT;//0;//
		for (int party=0;party<numOfParties;party++)
		  cyc->regularGates[g]->output->superseed[party] = RAND;
		cyc->regularGates[g]->output->seed=cyc->regularGates[g]->output->superseed[partyNum];
	}

	for (int g = 0; g < cyc->numOfXORGates; g++)
	{

		cyc->specialGates[g]->output->lambda = cyc->specialGates[g]->input1->lambda ^ cyc->specialGates[g]->input2->lambda;//out=in1^in2

		cyc->specialGates[g]->output->superseed=XORsuperseeds(cyc->specialGates[g]->input1->superseed, cyc->specialGates[g]->input2->superseed);

		cyc->specialGates[g]->output->seed=_mm_xor_si128(cyc->specialGates[g]->input1->seed, cyc->specialGates[g]->input2->seed);
	}


	for (int w=0;w<cyc->amountOfWire;w++)
	{
	  cyc->allWires[w].realLambda=cyc->allWires[w].lambda;
	}

	bigMemoryAllocation = static_cast<__m128i *>(_aligned_malloc(4*cyc->numOfANDGates*numOfParties * sizeof(__m128i)+numOfOutputWires, 16));

	Gate* gate;

	__m128i* arr = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));

	for (int g = 0; g < cyc->numOfANDGates; g++)
	{
	  gate = cyc->regularGates[g];
	  for (int j=0;j<4;j++)
	  {
	    gate->G[j] = &bigMemoryAllocation[(4*g+j)*numOfParties];
	    memcpy(gate->G[j],gate->output->superseed,numOfParties* sizeof(__m128i));
	  }

	  __m128i* input1Seed;
	  __m128i* input1OtherSeed = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
	  input1Seed=gate->input1->superseed;
	  XORsuperseedsNew(input1Seed,Rvec,input1OtherSeed);
	  __m128i* input2Seed;
	  __m128i* input2OtherSeed = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
	  input2Seed=gate->input2->superseed;
	  XORsuperseedsNew(input2Seed,Rvec,input2OtherSeed);


	  for (int p=0;p<numOfParties;p++)
	  {
	    PSEUDO_RANDOM_FUNCTION(input1Seed[p], input2Seed[p], gate->gateNumber, numOfParties, arr);

		XORsuperseedsNew(gate->G[0], arr, gate->G[0]);

		PSEUDO_RANDOM_FUNCTION(input1Seed[p], input2OtherSeed[p], gate->gateNumber, numOfParties, arr);

		XORsuperseedsNew(gate->G[1], arr, gate->G[1]);

		PSEUDO_RANDOM_FUNCTION(input1OtherSeed[p], input2Seed[p], gate->gateNumber, numOfParties, arr);

	    XORsuperseedsNew(gate->G[2], arr, gate->G[2]);

	    PSEUDO_RANDOM_FUNCTION(input1OtherSeed[p], input2OtherSeed[p], gate->gateNumber, numOfParties, arr);

	    XORsuperseedsNew(gate->G[3], arr, gate->G[3]);

	  }

	  int sh=gate->sh;

	  bool lambdaIn1=gate->input1->lambda;
	  bool lambdaIn2=gate->input2->lambda;
	  bool lambdaOut=gate->output->lambda;
	  int row=2*lambdaIn1+lambdaIn2;
	  row=3-row;

	  XORsuperseedsNew(gate->G[sh^row], Rvec, gate->G[sh^row]);
	  if (lambdaOut)
	    for (int i=0;i<4;i++)
	      XORsuperseedsNew(gate->G[sh^i], Rvec, gate->G[sh^i]);
 	}

	for (int p = 0; p < numOfParties; p++)
	    {
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			if (cyc->playerArray[p].playerWires[i]->lambda)
			  XORsuperseedsNew(cyc->playerArray[p].playerWires[i]->superseed, Rvec, cyc->playerArray[p].playerWires[i]->superseed);
		}
	    }



	cout<<"end of fake generation"<<endl;
}

//TODO: implement method if want to test input values != 0
void fakeInputSuperseeds()
{
	//Do nothing - superseeds for 0 value already in place
	//(need to change if want other input values)
}

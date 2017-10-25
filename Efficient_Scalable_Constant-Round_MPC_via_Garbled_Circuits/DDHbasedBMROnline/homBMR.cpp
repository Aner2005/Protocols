#include "homBMR.h"

//the circuit
Circuit* cyc;

//number of wires
int numOfWires;
int numOfInputWires;
int numOfOutputWires;

//number of gates
int numOfGates;

//number of players
int numOfParties;
//this player number
int partyNum;

//load the circuit
Circuit* loadCircuitHom(char* filename)
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
	numOfWires = cyc->amountOfWire;
	numOfGates=cyc->amountOfGates;
	return cyc;
}


//online phase - compute output
bool* computeOutputsHom()
{

	bool* outputs = new bool[cyc->outputWires.playerBitAmount];
	bool valueIn1;
	bool valueIn2;
	ZZ keyIn1;
	ZZ keyIn2;
	int row;
	ZZ gateValue;
	ZZ dec;
	Gate* gate;

 	for (int g = 0; g < cyc->amountOfGates; g++)
 	{
		gate=&cyc->gateArray[g];
		valueIn1=gate->input1->externalValue;
		valueIn2=gate->input2->externalValue;
		keyIn1=gate->input1->superseed;
		keyIn2=gate->input2->superseed;

		row=2*valueIn1+valueIn2;
		gateValue=gate->G[row];

		ZZ key;
		wrapper.sumZp(key,keyIn1,keyIn2);
#ifdef PRECOMPUTE
		decryptWprecomputation(dec,cyc->generators[4*(gate->gateFanOutNum-1)+row],key,gateValue);
#else
		decrypt(dec,4*g+row,key,gateValue);
#endif

#ifdef DLSE
		SqrRoot(dec,dec);
#else
		wrapper.extractRoot(dec,dec);
#endif

		gate->output->externalValue=wrapper.getLSB(dec);

		if (gate->output->externalValue)
			wrapper.subtractZp(dec,dec,wrapper.ONE);

		wrapper.multiplyZp(gate->output->superseed,dec,wrapper.INV_TWO);

 	}

 	for (int w = 0; w < cyc->outputWires.playerBitAmount; w++)
	{
 		outputs[w] = getTrueValueHom(cyc->outputWires.playerWires[w]);
	}

	return outputs;
#undef BMR_DEBUG
}


//fake (i.e., local) circuit generation
void generateFakeGarbledCircuitHom(Circuit* c, int partyNumber)
{

	partyNum=partyNumber;

	//generate fake lambdas and keys
	Wire* wire;
	for (int w=0;w<numOfWires;w++)
	{
		wire=&cyc->allWires[w];
		wire->fanout=0;
		wire->lambda = RAND_BIT;

		wire->keyZero = RAND;
		wire->keyOne = RAND;
		if (wire->lambda)
			wire->superseed=wire->keyOne;
		else
			wire->superseed=wire->keyZero;
	}

	bool lambdaIn1;
	bool lambdaIn2;
	bool lambdaOut;
	Gate* gate;

#ifdef PRECOMPUTE
	int maxfanout=0;
	for (int g = 0; g < cyc->amountOfGates; g++)
 	{
		gate=&cyc->gateArray[g];
		gate->input1->fanout++;
		gate->input2->fanout++;

		if (gate->input1->fanout>maxfanout)
		 		maxfanout=gate->input1->fanout;
		if (gate->input2->fanout>maxfanout)
		 		maxfanout=gate->input2->fanout;
	}

	for (int w=0;w<numOfWires;w++)
	{
		wire=&cyc->allWires[w];
		wire->usedFanouts=new int[maxfanout*maxfanout];
		for (int i = 0; i < maxfanout; i++) {
			wire->usedFanouts[i]=-1;
		}
	}
	int numOfGenerators=0;
	for (int g = 0; g < cyc->amountOfGates; g++)
 	{
		gate=&cyc->gateArray[g];
		bool tmp=true;
		int maxmin=0;
		while (tmp)
		{
			tmp=false;
			for (int i = 0; i < maxfanout*maxfanout; i++)
			{
				if (gate->input1->usedFanouts[i]==maxmin || gate->input2->usedFanouts[i]==maxmin)
				{
					tmp=true;
					maxmin++;
				}
			}
		}

		gate->gateFanOutNum=maxmin;
		if (maxmin>numOfGenerators)
			numOfGenerators=maxmin;
		for (int i = 0; i < maxfanout*maxfanout; i++)
		{
			if (gate->input2->usedFanouts[i]==-1)
			{
				gate->input2->usedFanouts[i]=maxmin;
				break;
			}
		}
		for (int i = 0; i < maxfanout*maxfanout; i++)
		{
			if (gate->input1->usedFanouts[i]==-1)
			{
				gate->input1->usedFanouts[i]=maxmin;
				break;
			}
		}
	}
	for (int w=0;w<numOfWires;w++)
	{
		wire=&cyc->allWires[w];
		delete[] wire->usedFanouts;
	}

	//4 generators for each gate
	numOfGenerators*=4;

	cyc->generators=new ZZPrecomputeExp*[numOfGenerators];
	ZZ* temp= new ZZ[numOfGenerators];
	wrapper.getRandomGenerators(temp,numOfGenerators);
	for (int i = 0; i < numOfGenerators; i++)
	{
		cyc->generators[i]=new ZZPrecomputeExp();
#ifdef DLSE
		cyc->generators[i]->initDLSE(temp[i],wrapper.getP(),DLSECONST);
#else
		cyc->generators[i]->init(temp[i],wrapper.getP());
#endif
		cyc->generators[i]->prepExpTableMSB();
	}
#endif

	for (int g=0;g<numOfGates;g++)
	{
		gate=&cyc->gateArray[g];


		lambdaIn1=gate->input1->lambda;
		lambdaIn2=gate->input2->lambda;
		lambdaOut=gate->output->lambda;
		bool externalValues[4];

		for (int row=0;row<4;row++)
		{

			if (computeExtVal(gate->truthTable,lambdaIn1,lambdaIn2,row)==lambdaOut)
				externalValues[row]=0;
			else
				externalValues[row]=1;

			if (externalValues[row])
			{
				gate->G[row]=gate->output->keyOne;
				wrapper.multiplyZp(gate->G[row],gate->G[row],wrapper.TWO);
				wrapper.sumZp(gate->G[row],gate->G[row],wrapper.ONE);
			}
			else
			{
				gate->G[row]=gate->output->keyZero;
				wrapper.multiplyZp(gate->G[row],gate->G[row],wrapper.TWO);
			}

			wrapper.multiplyZp(gate->G[row],gate->G[row],gate->G[row]);
			ZZ key;
			switch(row)
			{
				case 0:
					wrapper.sumZp(key, gate->input1->keyZero, gate->input2->keyZero);
					break;
				case 1:
					wrapper.sumZp(key, gate->input1->keyZero, gate->input2->keyOne);
					break;
				case 2:
					wrapper.sumZp(key, gate->input1->keyOne, gate->input2->keyZero);
					break;
				case 3:
				default:
					wrapper.sumZp(key, gate->input1->keyOne, gate->input2->keyOne);
					break;
			}

#ifdef PRECOMPUTE
			encryptWprecomputation(gate->G[row],cyc->generators[(gate->gateFanOutNum-1)*4+row],key,gate->G[row]);
#else
			encrypt(gate->G[row],4*g+row,key,gate->G[row]);
#endif
		}
	}

	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			cyc->playerArray[p].playerWires[i]->externalValue=cyc->playerArray[p].playerWires[i]->lambda;//
			cyc->playerArray[p].playerWires[i]->realLambda=cyc->playerArray[p].playerWires[i]->lambda;
		}
	}
	for (int w = 0; w < cyc->outputWires.playerBitAmount; w++)
	{
		cyc->outputWires.playerWires[w]->realLambda=cyc->outputWires.playerWires[w]->lambda;
	}

	cout<<"end of fake generation"<<endl;
}


void fakeInputSuperseedsHom()
{
	//Do nothing (always use zero inputs).
	//need to create methods for other inputs.
}

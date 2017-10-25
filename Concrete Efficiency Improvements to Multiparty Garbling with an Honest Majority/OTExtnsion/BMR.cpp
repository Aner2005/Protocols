/*
 * 	BMR.cpp
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016
 *
 */

#include "BMR.h"
#include <thread>
#include <mutex>
#include "secCompMultiParty.h"


//Threads
mutex mtxVal;
mutex mtxPrint;

//the circuit
Circuit* cyc;

//number of Wires
//int numOfWires;
int numOfInputWires;
int numOfOutputWires;

//number of players
int numOfParties;
//this player number
int partyNum;

//*********//
int numOfPartiesLCNew;
//*********//

//communication
string * addrs;
BmrNet ** communicationSenders;
BmrNet ** communicationReceivers;

//A random 128 bit number, representing the difference between the seed for value 0 and value 1 of the wires ("free XOR")
__m128i R;

//for exchanging data
__m128i** playerSeeds;

//Preallocate memory
__m128i* bigMemoryAllocation;


//load the circuit
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

//setting up communication
void initCommunication(string addr, int port, int player, int mode)
{
	char temp[25];
	strcpy(temp, addr.c_str());
	if (mode == 0)
	{
		communicationSenders[player] = new BmrNet(temp, port);
		communicationSenders[player]->connectNow();
	}
	else
	{
		communicationReceivers[player] = new BmrNet(port);
		communicationReceivers[player]->listenNow();
	}
}

void initializeCommunication(int* ports)
{
	int i;
	communicationSenders = new BmrNet*[numOfParties];
	communicationReceivers = new BmrNet*[numOfParties];
	thread *threads = new thread[numOfParties * 2];
	for (i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2 + 1] = thread(initCommunication, addrs[i], ports[i * 2 + 1], i, 0);
			threads[i * 2] = thread(initCommunication, "127.0.0.1", ports[i * 2], i, 1);
		}
	}
	for (int i = 0; i < 2 * numOfParties; i++)
	{
		if (i != 2 * partyNum && i != (2 * partyNum + 1))
			threads[i].join();//wait for all threads to finish
	}

	delete[] threads;
}

void initializeCommunicationSerial(int* ports)//Use this for many parties
{
	int i;
	communicationSenders = new BmrNet*[numOfParties];
	communicationReceivers = new BmrNet*[numOfParties];
	for (i = 0; i < numOfParties; i++)
	{
		if (i<partyNum)
		{
		  initCommunication( addrs[i], ports[i * 2 + 1], i,0);
		  initCommunication("127.0.0.1", ports[i * 2], i, 1);
		}
		else if (i>partyNum)
		{
		  initCommunication("127.0.0.1", ports[i * 2], i, 1);
		  initCommunication( addrs[i], ports[i * 2 + 1], i,0);
		}
	}
}


void initializeCommunication(char* filename, Circuit* c, int p)
{
	cyc = c;
	FILE * f = fopen(filename, "r");
	partyNum = p;
	char buff[STRING_BUFFER_SIZE];
	char ip[STRING_BUFFER_SIZE];

	addrs = new string[numOfParties];
	int * ports = new int[numOfParties * 2];


	for (int i = 0; i < numOfParties; i++)
	{
		fgets(buff, STRING_BUFFER_SIZE, f);
		sscanf(buff, "%s\n", ip);
		addrs[i] = string(ip);

		ports[2 * i] = 8000 + i*numOfParties + partyNum;
		ports[2 * i + 1] = 8000 + partyNum*numOfParties + i;
	}

	fclose(f);
	//if (numOfParties>20)
	  initializeCommunicationSerial(ports);

	delete[] ports;
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
			else//letting only Party i choose the lambdas for his input wires.
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
	//compute seeds and lambdas on wires of outputs of XOR gates by XORing the values on the input wires.
	freeXorCompute();
	
}

//XOR gates - calculate seeds and lambdas
void freeXorCompute()
{
	for (int g = 0; g < cyc->numOfXORGates; g++)
	{
		//XOR lambdas
		cyc->specialGates[g]->output->lambda = cyc->specialGates[g]->input1->lambda ^ cyc->specialGates[g]->input2->lambda;//out=in1^in2

		//XOR seeds
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



void computeGates()
{
	if (majType==0)
		bigMemoryAllocation = static_cast<__m128i *>(_aligned_malloc(4*cyc->numOfANDGates*numOfParties * sizeof(__m128i)+numOfOutputWires, 16));
	else
		bigMemoryAllocation = static_cast<__m128i *>(_aligned_malloc((4 * cyc->numOfANDGates*numOfParties+numOfOutputWires) * sizeof(__m128i), 16));

	__m128i seed1, seed2;
	Gate* gate;
	for (int g = 0; g < cyc->numOfANDGates; g++)//add the seeds to Ag, Bg, Cg, and Dg
	{
		gate = cyc->regularGates[g];

		seed1 = gate->input1->seed;
		seed2 = gate->input2->seed;

		//Compute the pseudorandom functions for Ag,Bg,Cg,Dg
		for (int i = 0; i < 4; i++)
			gate->G[i] = &bigMemoryAllocation[(4*g+i)*numOfParties];

		PSEUDO_RANDOM_FUNCTION(seed1, seed2, gate->gateNumber, numOfParties, gate->G[0]);//in1,in2
		PSEUDO_RANDOM_FUNCTION(seed1, _mm_xor_si128(seed2, R), gate->gateNumber, numOfParties, gate->G[1]);//in1,in2^R
		PSEUDO_RANDOM_FUNCTION(_mm_xor_si128(seed1, R), seed2, gate->gateNumber, numOfParties, gate->G[2]);//in1^R,in2
		PSEUDO_RANDOM_FUNCTION(_mm_xor_si128(seed1, R), _mm_xor_si128(seed2, R), gate->gateNumber, numOfParties, gate->G[3]);//in1^r,in2^R


		gate->G[0][partyNum] = _mm_xor_si128(gate->G[0][partyNum], gate->output->seed);
		gate->G[1][partyNum] = _mm_xor_si128(gate->G[1][partyNum], gate->output->seed);
		gate->G[2][partyNum] = _mm_xor_si128(gate->G[2][partyNum], gate->output->seed);
		gate->G[3][partyNum] = _mm_xor_si128(gate->G[3][partyNum], gate->output->seed);
	}
}


void sendInput(bool* inputs, int player)
{
	communicationSenders[player]->sendMsg(inputs, cyc->playerArray[partyNum].playerBitAmount);// +numOfOutputWires);
}

void receiveInputs(int player)
{
	bool* playerInputs = new bool[cyc->playerArray[player].playerBitAmount];

	communicationReceivers[player]->reciveMsg(playerInputs, cyc->playerArray[player].playerBitAmount);

	for (int i = 0; i < cyc->playerArray[player].playerBitAmount; i++)//store value in correct place
	{
		cyc->playerArray[player].playerWires[i]->value = playerInputs[i];
	}

	delete[] playerInputs;//free memory
}


void exchangeInputs()//send and receive inputs (XORed with lambda)
{
	bool *toSend = new bool[cyc->playerArray[partyNum].playerBitAmount];// +numOfOutputWires];
	for (int i = 0; i < cyc->playerArray[partyNum].playerBitAmount; i++)
	{
		//the value to send/use for calculations is the real value XORed with lambda
		toSend[i] = cyc->playerArray[partyNum].playerWires[i]->value;
	}

	thread *threads = new thread[numOfParties * 2];
	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2] = thread(sendInput, toSend, i);//send inputs to player i
			threads[i * 2 + 1] = thread(receiveInputs, i);//receive inputs from player i
		}
	}


	for (int i = 0; i < 2 * numOfParties; i++)
	{
		if (i != 2 * partyNum && i != (2 * partyNum + 1))
			threads[i].join();//wait for all threads to finish
	}

	delete[] threads;

	delete[] toSend;
}


void sendSeed(__m128i* inputs, int player)
{
	communicationSenders[player]->sendMsg(inputs, sizeof(__m128i)*numOfInputWires);
}

void receiveSeed(int player)
{
	__m128i* playerSeeds = static_cast<__m128i *>(_aligned_malloc(numOfInputWires*sizeof(__m128i), 16));//allocate temporary memory to store response
	communicationReceivers[player]->reciveMsg(playerSeeds, sizeof(__m128i)*numOfInputWires);//receive values from player

	int count = 0;
	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			cyc->playerArray[p].playerWires[i]->superseed[player] = playerSeeds[count];
			count++;
		}
	}
	_aligned_free(playerSeeds);//free memory
}


void exchangeSeeds()
{
	__m128i *toSend = static_cast<__m128i *>(_aligned_malloc(numOfInputWires*sizeof(__m128i), 16));//assign space;
	int count = 0;
	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			toSend[count] = cyc->playerArray[p].playerWires[i]->seed;
			if (cyc->playerArray[p].playerWires[i]->value)
				toSend[count] = _mm_xor_si128(toSend[count], R);
			cyc->playerArray[p].playerWires[i]->superseed[partyNum] = toSend[count];//update local superseed
			count++;
		}
	}

	thread *threads = new thread[numOfParties * 2];
	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2] = thread(sendSeed,toSend,i);//send inputs to player i
			threads[i * 2 + 1] = thread(receiveSeed, i);//receive inputs from player i
		}
	}
	for (int i = 0; i < 2 * numOfParties; i++)
	{
		if (i != 2 * partyNum && i != (2 * partyNum + 1))
		{
			threads[i].join();//wait for all threads to finish
		}
	}


	delete[] threads;
	_aligned_free(toSend);//free memory
}

//online phase - compute output
bool* computeOutputs()
{
	__m128i* arr = static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
	bool* outputs = new bool[cyc->outputWires.playerBitAmount];
	bool valueIn1;
	bool valueIn2;
	for (int g = 0; g < cyc->amountOfGates; g++)
	{

		if (cyc->gateArray[g].flagNOMUL)//free XOR
		{
			XORsuperseedsNew(cyc->gateArray[g].input1->superseed, cyc->gateArray[g].input2->superseed, cyc->gateArray[g].output->superseed);
		}
		else//Regular gate computation
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

//memory release functions

void deletePartial()
{
	_aligned_free(bigMemoryAllocation);

	freeCircuitPartial(cyc);

		deletePartialBGW();
}

void deleteAll()
{

	deleteAllBGW();

	//close connection
	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			delete communicationReceivers[i];
			delete communicationSenders[i];
		}
	}
	delete[] communicationReceivers;
	delete[] communicationSenders;

	delete[] addrs;

	//delete Circuit
	freeCircle(cyc);

	AES_free();
}

//synchronization functions
void sendBit(int player, bool* toSend)
{
	communicationSenders[player]->sendMsg(toSend, 1);
}

void receiveBit(int player)
{
	bool *sync = new bool[1];
	communicationReceivers[player]->reciveMsg(sync,1);
}

void synchronize()
{
	bool* toSend=new bool[1];
	toSend[0] = 0;
	thread *threads = new thread[numOfParties * 2];
	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2] = thread(sendBit, i, toSend);//send inputs to player i
			threads[i * 2 + 1] = thread(receiveBit, i);//receive inputs from player i
		}
	}
	for (int i = 0; i < 2 * numOfParties; i++)
	{
		if (i != 2 * partyNum && i != (2 * partyNum + 1))
		{
			threads[i].join();//wait for all threads to finish
		}
	}
	delete[] threads;
}



//*******************************************************************//

void exchangeSeedsOneEvaluator()
{
	__m128i *toSend = static_cast<__m128i *>(_aligned_malloc(numOfInputWires*sizeof(__m128i), 16));//assign space;
	int count = 0;
	for (int p = 0; p < numOfParties; p++)
	{
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			toSend[count] = cyc->playerArray[p].playerWires[i]->seed;
			if (cyc->playerArray[p].playerWires[i]->value)
				toSend[count] = _mm_xor_si128(toSend[count], R);
			cyc->playerArray[p].playerWires[i]->superseed[partyNum] = toSend[count];//update local superseed
			count++;
		}
	}

	if (partyNum!=0)
	{
		sendSeed(toSend,0);
	}
	else
	{
		thread *threads = new thread[numOfParties];
		for (int i = 0; i < numOfParties; i++)
		{
			if (i != partyNum)
			{
				threads[i] = thread(receiveSeed, i);//receive inputs from player i
			}
		}
		for (int i = 0; i < numOfParties; i++)
		{
			if (i != partyNum )
			{
				threads[i].join();//wait for all threads to finish
			}
		}
		delete[] threads;
	}

	_aligned_free(toSend);//free memory
}

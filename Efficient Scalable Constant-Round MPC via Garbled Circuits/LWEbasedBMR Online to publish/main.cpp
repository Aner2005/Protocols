#include <iostream>
#include <set>
#include <chrono>
#include <time.h>
#include "LWEWrapper.h"
#include "secCompMultiParty.h"




bool* computeOutputsHom(Circuit* c, LWEWrapper* wrapper);
void generateFakeGarbledCircuitHom(Circuit* c, int partyNumber, LWEWrapper* wrapper);

int main(int argc, char ** argv)
{
double offlineTimes[TEST_ROUNDS];
double onlineTimes[TEST_ROUNDS];


//honest majority type (not used), player number
int hm,partyNum;
//circuit
Circuit* c;
//the output
bool *out;
//for encryption/decryption
LWEWrapper* wrapper;
GaussianSampler* sampler=new GaussianSampler();

int accCompute=0,repetition=0;
{
	//Start initialization phase


	partyNum = atoi(argv[1]);//Use this to get player number from arguments (use for local run).

	if (partyNum < 0)
	   partyNum=0;

	cout << "This is player:" << partyNum << endl;



#ifdef PRINT_STEPS
	cout << "Reading circuit from file" << endl;
#endif

	//1 - Read the circuit from the file
	c = readCircuitFromFile(argv[2]);

	cout << "Number parties: " << c->amountOfPlayers << endl;
	//cout << "Number of iterations: " << TEST_ROUNDS << endl;
    wrapper = new LWEWrapper(c->amountOfPlayers);

#ifdef PRINT_STEPS
	cout << endl<<"User given key "<< argv[5] << endl;
#endif

	initializeRandomnessHom(argv[5], c->amountOfPlayers);

	//check adversary model
	hm = 0;
	if (argc > 6) hm = atoi(argv[6]);

}




 for (int rep=0; rep<TEST_ROUNDS; rep++)
 {
 	//NEW - fake offline phase
    auto generateStartTime = chrono::high_resolution_clock::now();

 	generateFakeGarbledCircuitHom(c, partyNum, wrapper);

    auto generateEndTime = chrono::high_resolution_clock::now();
    chrono::duration<double> generateTotalTime = generateEndTime - generateStartTime;
    offlineTimes[rep]=generateTotalTime.count();
    cout<<"Fake generation time: "<<generateTotalTime.count() <<endl;
//
//
 	{//Start of online phase
//
// 	//2 - load inputs from file - not implemented - always zero value inputs
 	//loadInputsHom(argv[3], c, partyNum);
//
// 	//NEW - get input superseeds - not implemented - always keys corresponding to (real) zero input values.
 	//fakeInputSuperseedsHom();
//
 #ifdef PRINT_STEPS
 		cout << "Computing values of garbled circuit" << endl;
 #endif
 		auto computeStartTime = chrono::high_resolution_clock::now();
// 		//11 - compute output
 		out = computeOutputsHom(c,wrapper);
//
 		auto computeEndTime = chrono::high_resolution_clock::now();
 		chrono::duration<double> computeTotalTime = computeEndTime - computeStartTime;
        onlineTimes[rep]=computeTotalTime.count();
// 		if (repetition>0)
// 			accCompute += computeTotalTime.count();
//
//
#ifdef PRINT_STEPS
 		cout << "Outputs computed" << endl;
#endif
//
//
 	cout << "Outputs:" << endl;
//
 	for (int i = 0; i < c->outputWires.playerBitAmount; i++)
 	{
 		cout << out[i];
 	}
 	cout<<endl;
//
 	cout<<"Homomorphic Online computation time: "<<computeTotalTime.count() <<endl;
 	if (repetition < TEST_ROUNDS - 1)
 	  delete[] out;
 	}
//
// 	deletePartial();
 }//end for loop
// 	//deleteAll();


    return 0;
}


bool* computeOutputsHom(Circuit* circuit,LWEWrapper* wrapper)
{

	bool* outputs = new bool[circuit->outputWires.playerBitAmount];
	bool valueIn1;
	bool valueIn2;
	uint32_t* keyIn1;
	uint32_t* keyIn2;
	int row;
	uint32_t* gateValue;
	uint32_t* dec;
	Gate* gate;


 	for (int g = 0; g < circuit->amountOfGates; g++)
 	{
		gate=&circuit->gateArray[g];
		valueIn1=gate->input1->externalValue;
		valueIn2=gate->input2->externalValue;
		keyIn1=gate->input1->superseed;
		keyIn2=gate->input2->superseed;
		//choose row using input external values
		row=2*valueIn1+valueIn2;
		gateValue=gate->G[row];

		//*****
		uint32_t key[M];
		wrapper->sumKeys(keyIn1,keyIn2,key);

        wrapper->fwd_ntt_r(key);
        wrapper->decrypt(gateValue, circuit->publicElements[4*(gate->gateFanOutNum-1)+row], key, gate->output->superseed, gate->output->externalValue);

 	}
 	for (int w = 0; w < circuit->outputWires.playerBitAmount; w++)
	{
 		outputs[w] = getTrueValueHom(circuit->outputWires.playerWires[w]);
	}

	return outputs;
#undef BMR_DEBUG
}

//NEW - fake generation
void generateFakeGarbledCircuitHom(Circuit* circuit, int partyNumber, LWEWrapper* wrapper)
{
	//generate fake lambdas and keys
	Wire* wire;
	for (int w=0;w<circuit->amountOfWire;w++)
	{
		wire=&circuit->allWires[w];
		wire->fanout=0;
		wire->lambda = RAND_BIT;

        wire->keyZero= new uint32_t[M];
		wrapper->error_gen(wire->keyZero);
        wire->keyZero[0]=0;
        wire->keyOne= new uint32_t[M];
        wrapper->error_gen(wire->keyOne);
        wire->keyOne[0]=0;
        wire->superseed= new uint32_t[M];
        for (int i=0;i<M;i++)
        {
    		if (wire->lambda)
    			wire->superseed[i]=wire->keyOne[i];
    		else
    			wire->superseed[i]=wire->keyZero[i];
        }
	}

	bool lambdaIn1;
	bool lambdaIn2;
	bool lambdaOut;
	Gate* gate;


	int maxfanout=0;
	for (int g = 0; g < circuit->amountOfGates; g++)
 	{
		gate=&circuit->gateArray[g];
		gate->input1->fanout++;
		gate->input2->fanout++;

		if (gate->input1->fanout>maxfanout)
		 		maxfanout=gate->input1->fanout;
		if (gate->input2->fanout>maxfanout)
		 		maxfanout=gate->input2->fanout;
	}

	for (int w=0;w<circuit->amountOfWire;w++)
	{
		wire=&circuit->allWires[w];
		wire->usedFanouts=new int[maxfanout*maxfanout];
		for (int i = 0; i < maxfanout; i++) {
			wire->usedFanouts[i]=-1;
		}
	}
	int numOfGenerators=0;
	for (int g = 0; g < circuit->amountOfGates; g++)
 	{
		gate=&circuit->gateArray[g];
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
	for (int w=0;w<circuit->amountOfWire;w++)
	{
		wire=&circuit->allWires[w];
		delete[] wire->usedFanouts;
	}

	//4 generators for each gate
	numOfGenerators*=4;

	cout <<"needed number of generators is "<<numOfGenerators<<endl;
	circuit->publicElements=new uint32_t*[numOfGenerators];

	for (int i = 0; i < numOfGenerators; i++)
	{
		circuit->publicElements[i]=new uint32_t[M];
        wrapper->pub_key_gen(circuit->publicElements[i]);//********************
	}

	//compute values to encrypt and encrypt
	for (int g=0;g<circuit->amountOfGates;g++)
	{

		gate=&circuit->gateArray[g];

		lambdaIn1=gate->input1->lambda;
		lambdaIn2=gate->input2->lambda;
		lambdaOut=gate->output->lambda;
		bool externalValues[4];

		for (int row=0;row<4;row++)
		{
			//compute key and external value to encrypt
			if (computeExtVal(gate->truthTable,lambdaIn1,lambdaIn2,row)==lambdaOut)
				externalValues[row]=0;
			else
				externalValues[row]=1;
            gate->G[row]=new uint32_t[M];
			if (externalValues[row])
			{
				for (int i = 0; i < M; i++)
                {
				    gate->G[row][i]=gate->output->keyOne[i];
				}
                gate->G[row][0]=1;

			}
			else
			{
                for (int i = 0; i < M; i++)
                {
				    gate->G[row][i]=gate->output->keyZero[i];
				}
                gate->G[row][0]=0;

			}

			uint32_t key[M];
			switch(row)
			{
				case 0:
                    wrapper->sumKeys(gate->input1->keyZero, gate->input2->keyZero,key);
					break;
				case 1:
                    wrapper->sumKeys(gate->input1->keyZero, gate->input2->keyOne,key);
					break;
				case 2:
                    wrapper->sumKeys(gate->input1->keyOne, gate->input2->keyZero,key);
					break;
				case 3:
				default:
                    wrapper->sumKeys(gate->input1->keyOne, gate->input2->keyOne,key);
					break;
			}

            wrapper->fwd_ntt_r(key);
			wrapper->encrypt(gate->G[row],circuit->publicElements[(gate->gateFanOutNum-1)*4+row],key,gate->G[row]);//************TODO:fix

		}
	}

	for (int p = 0; p < circuit->amountOfPlayers; p++)
	{
		for (int i = 0; i < circuit->playerArray[p].playerBitAmount; i++)
		{
			circuit->playerArray[p].playerWires[i]->externalValue=circuit->playerArray[p].playerWires[i]->lambda;//
			circuit->playerArray[p].playerWires[i]->realLambda=circuit->playerArray[p].playerWires[i]->lambda;
		}
	}
	for (int w = 0; w < circuit->outputWires.playerBitAmount; w++)
	{
		circuit->outputWires.playerWires[w]->realLambda=circuit->outputWires.playerWires[w]->lambda;
	}

	cout<<"end of fake generation"<<endl;
}

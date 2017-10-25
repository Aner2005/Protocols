/*
 * 	Online Test - locally creates a garbled circuit corresponding
 *  to any number of parties and then decrypts it according to all
 *  zeros inputs (need to modify fakeInputSuperseeds for other inputs)
 *
 *  File purpose is for timing of online phase of semi-honest BMR implementation
 *  [Ben-Efraim, Lindell, Omri ACM CCS 2016] only!!!
 *
 *  Similar file used for timing in [Ben-Efraim, Lindell, Omri Asiacrypt 2017]
 *
 *      Author: Aner Ben-Efraim
 *
 * 	year: 2016 & 2017
 *
 */

#include <thread>
#include "secCompMultiParty.h"
#include "BMR.h"
#include <math.h>

using namespace std;

/*main program. Should receive as arguments:
1. A file specifying the circuit, number of players, input wires, output wires
2. A file specifying the connection details
3. A file specifying the current player, inputs
4. Player number
5. Random seed
*/
int main(int argc, char** argv)
{

double offlineTimes[TEST_ROUNDS];
double onlineTimes[TEST_ROUNDS];
//honest majority type, player number
int hm,p;
//circuit
Circuit* c;
//the output
bool *out;
//#define PRINT_STEPS
int accCompute=0,repetition=0;
{
	//Start initialization phase

	p = 0;//partynum not important (needs to be legal participating party)

	cout << "This is party:" << p << endl;



#ifdef PRINT_STEPS
	cout << "Reading circuit from file" << endl;
#endif

	//1 - Read the circuit from the file
	c = loadCircuit(argv[2]);

	cout << "Number parties: " << c->amountOfPlayers << endl;
	cout << "Number of AND gates: " << c->numOfANDGates << endl;
	cout << "Number of XOR gates: " << c->numOfXORGates << endl;
	//cout << "Number of iterations: " << TEST_ROUNDS << endl;

#ifdef PRINT_STEPS
	cout << endl<<"User given key "<< argv[5] << endl;
#endif

	initializeRandomness(argv[5], c->amountOfPlayers);

	//check adversary model
	hm = 0;
	if (argc > 6) hm = atoi(argv[6]);

	VERSION(hm);
}

for (int rep=0;rep < TEST_ROUNDS;rep++)
{
	//offline phase - (fake, i.e., local circuit generation, no communication)
	auto generateStartTime = chrono::high_resolution_clock::now();

	generateFakeGarbledCircuit(c, p);

	auto generateEndTime = chrono::high_resolution_clock::now();
	chrono::duration<double> generateTotalTime = generateEndTime - generateStartTime;
	offlineTimes[rep]=generateTotalTime.count();
	cout<<"Fake generation time: "<<generateTotalTime.count() <<endl;

	{//Start of online phase

	//2 - load inputs from file
	loadInputs(argv[3], c, p);

	//get input superseeds - (fake, i.e., no communication)
	fakeInputSuperseeds();

#ifdef PRINT_STEPS
		cout << "Computing values of garbled circuit" << endl;
#endif
		auto computeStartTime = chrono::high_resolution_clock::now();
		//11 - compute output
		out = computeOutputs();

		auto computeEndTime = chrono::high_resolution_clock::now();
		chrono::duration<double> computeTotalTime = computeEndTime - computeStartTime;
		onlineTimes[rep]=computeTotalTime.count();
		if (repetition>0)
			accCompute += computeTotalTime.count();


#ifdef PRINT_STEPS
		cout << "Outputs computed" << endl;
#endif


	cout << "Outputs:" << endl;

	for (int i = 0; i < cyc->outputWires.playerBitAmount; i++)
	{
		cout << out[i];
	}
	cout<<endl;

	cout<<"Online computation time: "<<computeTotalTime.count() <<endl;
	//if (repetition < TEST_ROUNDS - 1)
	  //delete[] out;
	}

	deletePartial();
}//end for loop

//deleteAll();

	return 0;
}

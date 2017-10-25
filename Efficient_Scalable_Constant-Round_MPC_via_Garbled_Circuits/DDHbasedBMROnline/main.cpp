#include <iostream>
#include <set>
#include <time.h>

typedef unsigned char byte;

#include "homBMR.h"
#include <chrono>


#define PRINT_PERF_STATS

using namespace std;
using namespace std::chrono ;

#include <openssl/sha.h>

int main(int argc, char ** argv)
{
double offlineTimes[TEST_ROUNDS];
double onlineTimes[TEST_ROUNDS];

NTLWrapper wrapper = NTLWrapper();
//honest majority type (not used), player number
int hm,partyNum;
//circuit
Circuit* c;
//the output
bool *out;
    cout<<"---------------------------------Using "<<(KAPPA+1) <<" bit prime-----------------------------------"<<endl;
#ifdef PRECOMPUTE
    cout<<"---------------------------------With Precomputation------------------------------------"<<endl;
#endif
#ifdef DLSE
    cout<<"---------------------------------Assuming DLSE "<<(DLSECONST-10)<<" bits---------------------------------"<<endl;
#endif


int accCompute=0,repetition=0;
{
	//Start initialization phase


	partyNum = atoi(argv[1]);//Use this to get player number from arguments (use for local run).

	if (partyNum < 0)
	   partyNum=0;

	cout << "This is party:" << partyNum << endl;



#ifdef PRINT_STEPS
	cout << "Reading circuit from file" << endl;
#endif

	//1 - Read the circuit from the file
	c = loadCircuitHom(argv[2]);

	cout << "Number parties: " << c->amountOfPlayers << endl;
	cout << "Number of AND gates: " << c->numOfANDGates << endl;
	cout << "Number of XOR gates: " << c->numOfXORGates << endl;

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

 	generateFakeGarbledCircuitHom(c, partyNum);

    auto generateEndTime = chrono::high_resolution_clock::now();
    chrono::duration<double> generateTotalTime = generateEndTime - generateStartTime;
    offlineTimes[rep]=generateTotalTime.count();
    cout<<"Fake generation time: "<<generateTotalTime.count() <<endl;
 	{//Start of online phase

// 	//2 - load inputs from file
// 	//3 - get input superseeds
	//Do nothing (always use zero inputs).
	//need to create methods for other inputs.

 #ifdef PRINT_STEPS
 		cout << "Computing values of garbled circuit" << endl;
 #endif
 		auto computeStartTime = chrono::high_resolution_clock::now();
 		//11 - compute output
 		out = computeOutputsHom();

 		auto computeEndTime = chrono::high_resolution_clock::now();
 		chrono::duration<double> computeTotalTime = computeEndTime - computeStartTime;
        onlineTimes[rep]=computeTotalTime.count();

#ifdef PRINT_STEPS
 		cout << "Outputs computed" << endl;
#endif

 	cout << "Outputs:" << endl;

 	for (int i = 0; i < c->outputWires.playerBitAmount; i++)
 	{
 		cout << out[i];
 	}
 	cout<<endl;

 	cout<<"Homomorphic Online computation time: "<<computeTotalTime.count() <<endl;
 	if (repetition < TEST_ROUNDS - 1)
 	  delete[] out;
 	}

 	//deletePartial();
 }//end for loop
 	//deleteAll();
    return 0;
}

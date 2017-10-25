#include <thread>
#include "secCompMultiParty.h"
#include "BMR.h"
#include "BMR_BGW.h"
//
#include "BMR_BGW_Latincrypt.h"
//
#include <math.h>

//LOCK
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

//version, player number
int version,p;
//circuit
Circuit* c;
//the output
bool *out;

inline void intializationPhase(int argc, char** argv);
inline void offlinePhase();
// inline void computeRandom();
inline void runBGWprotocol();
inline bool* onlinePhase(char* filename);
inline bool* onlinePhaseLatinCrypt(char* filename);
inline void runBGWprotocolLatinCrypt();
void writeToFile(char** argv, int version);


//#define PRINT_STEPS
#define TIMING

//For timing
int repetition;
double totalTimewSetup;
double setupTime;
double offlineTimes[TEST_ROUNDS];
double offlineIndTimes[TEST_ROUNDS];
double offlineDepTimes[TEST_ROUNDS];
double onlineTimes[TEST_ROUNDS];
//double onlineTimesSync[TEST_ROUNDS];
double computationTimes[TEST_ROUNDS];
double totalTimes[TEST_ROUNDS];//without setup

/*main program. Should receive as arguments:
1. A file specifying the circuit, number of players, input wires, output wires
2. A file specifying the connection details
3. A file specifying the current player, inputs
4. Player number
5. Random seed
*/
int main(int argc, char** argv)
{

	// donothingTest();
#ifdef TIMING
	//start Timing
	auto allStart = chrono::high_resolution_clock::now();//clock();
	//Start setup phase
	auto intializationPhaseStart = chrono::high_resolution_clock::now();//clock();
#endif
{
	//NEW VERSIONS (See versions list for details):
	//versions 10-15, optimized BGW3
	//versions 20-25, optimized BGW2
	intializationPhase(argc,argv);
}
#ifdef TIMING
	//End setup phase
	auto intializationPhaseEnd = chrono::high_resolution_clock::now();
	setupTime=(double) (chrono::duration<double> (intializationPhaseEnd - intializationPhaseStart)).count();
#ifdef PRINT_STEPS
	cout<<"Setup Time = "<<setupTime<<endl;
#endif //PRINT_STEPS
#endif //TIMING


for (repetition = 0; repetition < TEST_ROUNDS; repetition++)
{
	#ifdef TIMING
		auto roundStart = chrono::high_resolution_clock::now();
	#endif

		synchronize();
		synchronize();

	offlinePhase();


	{
	synchronize();
	synchronize();

	#ifdef TIMING
		//Start of online phase
		auto onlineStart = chrono::high_resolution_clock::now();
	#endif

	out=onlinePhaseLatinCrypt(argv[3]);


	#ifdef PRINT_STEPS
	cout<<endl;
	#endif //PRINT_STEPS

	#ifdef TIMING
		//end offline phase
		auto onlineEnd = chrono::high_resolution_clock::now();
		onlineTimes[repetition]=(double) (chrono::duration<double> (onlineEnd - onlineStart)).count();
	#ifdef PRINT_STEPS
		cout<<"Online Time = "<<onlineTimes[repetition]<<endl;
	#endif //PRINT_STEPS
	#endif //TIMING

	if (repetition < TEST_ROUNDS - 1)
		delete[] out;
	}

	deletePartial();

	#ifdef TIMING

	auto roundEnd = chrono::high_resolution_clock::now();
	totalTimes[repetition]=(double) (chrono::duration<double> (roundEnd - roundStart)).count();
	#ifdef PRINT_STEPS
		cout<<"Round Time = "<<totalTimes[repetition]<<endl;
	#endif //PRINT_STEPS
	#endif //TIMING

}//end for loop
#ifdef TIMING
	writeToFile(argv, version);
#endif
	deleteAll();
	return 0;
}


inline void intializationPhase(int argc, char** argv)
{
	p = atoi(argv[1]);//Use this to get player number from arguments (use for local run).

	if (p < 0)
	{
		p = getPartyNum(argv[4]);//Use this to get player number by matching IP address
	}

	cout << "This is player:" << p << endl;



#ifdef PRINT_STEPS
	cout << "Reading circuit from file" << endl;
#endif

	//1 - Read the circuit from the file
	c = loadCircuit(argv[2]);

	cout << "Number parties: " << c->amountOfPlayers << endl;
	cout << "Number of AND gates: " << c->numOfANDGates << endl;
	cout << "Number of XOR gates: " << c->numOfXORGates << endl;


#ifdef PRINT_STEPS
	cout << "Circuit read"<<endl;
	cout << "Initializing Communication" << endl;
#endif

	//2 - initialize the communication
	initializeCommunication(argv[4], c, p);

#ifdef PRINT_STEPS
	cout << "Communication Initialized" << endl;
#endif

#ifdef PRINT_STEPS
	cout << endl<<"User given key "<< argv[5] << endl;
#endif

	//check version
	version = atoi(argv[6]);

	VERSION(version);//print version type

	cout<<"Version number: "<<version<<endl;

	initializeRandomness(argv[5], c->amountOfPlayers);//Some small inefficiency here

	if (version==13 || version==14 || version==23 || version==24)
		initializePrivateKeys();//for zero-sharing without sending

	//initialize BGW based algorithm
	{
#ifdef PRINT_STEPS
		cout << "Initializing BGW coef" << endl;
#endif
		//3.2 - precompute powers and degree reduction coefficients used in BGW protocol.
		InitializeBGW(version);//Honest majority - we will use BGW
#ifdef PRINT_STEPS
		cout << "BGW coef initialized" << endl;
#endif
	}

}

inline void offlinePhase()//This is where the main changes are!
{
	#ifdef TIMING
		//Start offline phase
		auto offlineStart = chrono::high_resolution_clock::now();
	#endif


	#ifdef PRINT_STEPS
		cout << "Choosing Seeds and lambdas" << endl;
	#endif

		//4 - load random R, random seeds, random lambdas and compute lambdas/seed of XOR gates
		newLoadRandom();

	#ifdef PRINT_STEPS
		cout << "Seeds and lambdas chosen" << endl;

		//cout << "Ready to Roll" << endl;

		cout << "Computing AES for gates" << endl;
	#endif

		//here the regular gates go
		//5 - Preallocate memory and compute AES of Ag,Bg,Cg,Dg for multiplication gates
		computeGatesHME();

	#ifdef PRINT_STEPS
		cout << "AES for gates computed" << endl;
	#endif

	///*******************//
	#ifdef PRINT_STEPS
		cout << "Computing and distributing shares" << endl;
	#endif
		//5.2 Create secret-sharings of correct degrees and send/receive
		if (version!=12 && version != 13  && version != 14 && version != 22 && version != 23 && version != 24)
			secretShare();
		else
			secretShareHME();

	#ifdef TIMING
		//end offline phase
		auto preprocessingIndEnd = chrono::high_resolution_clock::now();
		offlineIndTimes[repetition]=(double) (chrono::duration<double> (preprocessingIndEnd - offlineStart)).count();
	#ifdef PRINT_STEPS
		cout<<"Circuit Independent Preprocessing Time = "<<offlineIndTimes[repetition]<<endl;
	#endif	//PRINT_STEPS
	#endif  //TIMING

	synchronize();
	synchronize();
	#ifdef TIMING
		//Start function depenedent offline phase
		auto preprocessingDepStart = chrono::high_resolution_clock::now();
	#endif

	#ifdef PRINT_STEPS
		cout << "Shares computed and distributed" << endl;

		cout << "Multiplying lambdas" << endl;
	#endif

		//6 - multiply the lambdas for multiplication gates - local computation
		mulLambdasBGW();//local computation
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Multiplied lambdas" << endl;
	#endif
		if (version != 2 && version < 20)//BGW 3 or BGW 4 - need to do degree reduction
		{
	#ifdef PRINT_STEPS
		cout << "Reducing Degree" << endl;
	#endif

		//6.2 - reduce degree of share polynomials (not needed if >2/3 honest)
		reduceDegBGW();
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Reduced Degree" << endl;
	#endif
		}

	#ifdef PRINT_STEPS
		cout << "Multiplying R" << endl;
	#endif

		if (version<10 || version == 10 || version == 20)
		{
			#ifdef PRINT_STEPS
				cout<<"Regular R multiplication"<<endl;
			#endif //PRINT_STEPS
			//7 - according to lambdas multiplication, multiply R for multiplication gates. Add results to Ag,Bg,Cg,Dg respectively.
			if (version != 3)
				mulRBGW();//local computation
			else//obsolete
				mulRBGWwithReduction();//run BGW4 with an extra reduction step
			//in BMR_BGW
		}
		else
		{
			#ifdef PRINT_STEPS
					cout<<"R multiplication with share conversion"<<endl;
			#endif //PRINT_STEPS
				mulRBGWwShareConversionHME();//local computation - with share conversion
		}

	#ifdef PRINT_STEPS
		cout << "Multiplied R" << endl;

		cout << "Exchanging gates" << endl;
	#endif
		if (version<10 )
		{
			//8 - send and receive gates
			exchangeGatesBGW();//communication round, also exchanges the output lambdas, and reconstructs the gates
			//in BMR_BGW
		}
		else if (version==10 || version ==20)
		{
			exchangeGatesBGWOneEvaluator();
		}
		else if (version != 14 && version != 24)//versions 11,12,13,21,22 & 23
		{
				//8 - send and receive gates

				exchangeGatesBGWLatinCryptHME();
		}

		else// version 14 or 24 - hypercube
		{
			//8 - send and receive gates in log n rounds
			#ifdef PRINT_STEPS
				cout<<"Last round in log n rounds"<<endl;
			#endif //PRINT_STEPS
			
			exchangeGatesBGWLatinCryptHMElognRounds();
		}

		#ifdef PRINT_STEPS
			cout << "Exchanged gates" << endl;
		#endif
	//*************************************//


	#ifdef TIMING
		//end offline phase
		auto offlineEnd = chrono::high_resolution_clock::now();
		offlineTimes[repetition]=(double) (chrono::duration<double> (offlineEnd - offlineStart)).count();
		offlineDepTimes[repetition]=(double) (chrono::duration<double> (offlineEnd - preprocessingDepStart)).count();
	#ifdef PRINT_STEPS
		cout<<"Offline Time = "<<offlineTimes[repetition]<<endl;
	#endif	//PRINT_STEPS
	#endif  //TIMING

}

inline void runBGWprotocol()//BGW versions except versions 12, 13, 22, and 23
{
	#ifdef PRINT_STEPS
		cout << "Computing and distributing shares" << endl;
	#endif
		//5.2 Create secret-sharings of correct degrees and send/receive
			secretShare();//secret-share lambdas, R, and PRGs - first communication round
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Shares computed and distributed" << endl;

		cout << "Multiplying lambdas" << endl;
	#endif

		//6 - multiply the lambdas for multiplication gates - local computation
		mulLambdasBGW();//local computation
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Multiplied lambdas" << endl;
	#endif
		if (version != 2 && version < 20)//BGW 3 or BGW 4 - need to do degree reduction
		{
	#ifdef PRINT_STEPS
		cout << "Reducing Degree" << endl;
	#endif

		//6.2 - reduce degree of share polynomials (not needed if >2/3 honest)
		reduceDegBGW();
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Reduced Degree" << endl;
	#endif
		}

	#ifdef PRINT_STEPS
		cout << "Multiplying R" << endl;
	#endif

		if (version<10 || version == 10 || version == 20)
		{
			#ifdef PRINT_STEPS
				cout<<"Regular R multiplication"<<endl;
			#endif //PRINT_STEPS
			//7 - according to lambdas multiplication, multiply R for multiplication gates. Add results to Ag,Bg,Cg,Dg respectively.
			if (version != 3)
				mulRBGW();//local computation
			else
				mulRBGWwithReduction();//run BGW4 with an extra reduction step
			//in BMR_BGW
		}
		else
		{
			#ifdef PRINT_STEPS
				cout<<"R multiplication with share conversion"<<endl;
			#endif //PRINT_STEPS
				mulRBGWwShareConversion();
		}

	#ifdef PRINT_STEPS
		cout << "Multiplied R" << endl;

		cout << "Exchanging gates" << endl;
	#endif
		if (version<10 )
		{
			//8 - send and receive gates
			exchangeGatesBGW();//communication round, also exchanges the output lambdas, and reconstructs the gates
			//in BMR_BGW
		}
		else if (version==15 || version==10 || version ==20)
		{
			exchangeGatesBGWOneEvaluator();
		}
		else
		{
			#ifdef PRINT_STEPS
				cout<<"With share conversion"<<endl;
			#endif //PRINT_STEPS
			exchangeGatesBGWLatinCrypt();
		}

		#ifdef PRINT_STEPS
			cout << "Exchanged gates" << endl;
		#endif
}


inline bool* onlinePhase(char* filename)
{
	//2 - load inputs from file
	loadInputs(filename, cyc, p);

#ifdef PRINT_STEPS
	cout << "Revealing masked inputs" << endl;
#endif
	//9 - reveal Inputs (XORed with lambda)
	exchangeInputs();

#ifdef PRINT_STEPS
	cout << "Revealed masked inputs" << endl;

	cout << "Revealing corresponding seeds" << endl;
#endif
	//10 - reveal corresponding seeds

	if (version<10)
		exchangeSeeds();
	else
		exchangeSeedsOneEvaluator();
#ifdef PRINT_STEPS
	cout << "Revealed corresponding seeds" << endl;
#endif

#ifdef PRINT_STEPS
	cout << "Computing values of garbled circuit" << endl;
#endif
	bool* outputs;
	//11 - compute output
	if (version <10 || p==0)//evaluating party
	{
		outputs = computeOutputs();

		#ifdef PRINT_STEPS
			cout << "Outputs computed" << endl;



			cout << "Outputs:" << endl;

			for (int i = 0; i < cyc->outputWires.playerBitAmount; i++)
			{
				cout << outputs[i];
			}
		#endif //PRINT_STEPS
	}
	else //not evaluating party
		outputs = new bool[cyc->outputWires.playerBitAmount];

	return outputs;

}
//*******************************************************************//


inline void runBGWprotocolLatinCrypt()
{
	#ifdef PRINT_STEPS
		cout << "Computing and distributing shares" << endl;
	#endif
		//5.2 Create secret-sharings of correct degrees and send/receive
		secretShareHME();//secret-share lambdas, R, and PRGs - first communication round
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Shares computed and distributed" << endl;

		cout << "Multiplying lambdas" << endl;
	#endif

		//6 - multiply the lambdas for multiplication gates - local computation
		mulLambdasBGW();//local computation
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Multiplied lambdas" << endl;
	#endif

	if (version!= 2 && version <20)
	{
	#ifdef PRINT_STEPS
		cout << "Reducing Degree" << endl;
	#endif

		//6.2 - reduce degree of share polynomials
		reduceDegBGW();
		//in BMR_BGW

	#ifdef PRINT_STEPS
		cout << "Reduced Degree" << endl;
	#endif
	}

	#ifdef PRINT_STEPS
		cout << "Multiplying R (with HME) and share conversion" << endl;
	#endif


	mulRBGWwShareConversionHME();

	#ifdef PRINT_STEPS
		cout << "Multiplied R" << endl;

		cout << "Exchanging gates" << endl;
	#endif
		if (version==12 || version ==22)
		{
			//8 - send and receive gates
			exchangeGatesBGWLatinCryptHME();
		}
		else// version 13 or 23
		{
			//8 - send and receive gates
			exchangeGatesBGWLatinCryptHMElognRounds();
		}
}

inline bool* onlinePhaseLatinCrypt(char* filename)
{
	//2 - load inputs from file
	loadInputs(filename, cyc, p);

#ifdef PRINT_STEPS
	cout << "Revealing masked inputs" << endl;
#endif
	//9 - reveal Inputs (XORed with lambda)
	exchangeInputs();

#ifdef PRINT_STEPS
	cout << "Revealed masked inputs" << endl;

	cout << "Revealing corresponding seeds" << endl;
#endif
	//10 - reveal corresponding seeds
	if (version<10)//all evaluating
		exchangeSeeds();
	else//One evaluator, possibly short superseed
	exchangeSeedsHME();
#ifdef PRINT_STEPS
	cout << "Revealed corresponding seeds" << endl;
#endif

#ifdef PRINT_STEPS
	cout << "Computing values of garbled circuit" << endl;
#endif

	bool* outputs;

	//11 - compute output
	if (p==0 || version<10)//evaluating party
	{
		outputs = computeOutputsHME();


	#ifdef PRINT_STEPS
			cout << "Outputs computed" << endl;



		cout << "Outputs:" << endl;

		for (int i = 0; i < cyc->outputWires.playerBitAmount; i++)
		{
			cout << outputs[i];
		}
	#endif //PRINT_STEPS
	}
	else //not evaluating party
		outputs = new bool[cyc->outputWires.playerBitAmount];//for delete

	return outputs;
}


void writeToFile(char** argv, int version)
{
	//writing to file not implemented - only writes output (of last run) to screen
	 if (p==0)//evaluating party
	 {
		 {
			 cout << "Outputs:" << endl;
	 		 for (int i = 0; i < cyc->outputWires.playerBitAmount; i++)
	 		 {
	 			cout << out[i];
	 		 }
		 }
		 delete[] out;
	 }
}

/*
* secCompMultiParty.cpp
*
*  Created on: Mar 31, 2015
*      Author: froike (Roi Inbar)
*      Modified: Aner Ben-Efraim
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "secCompMultiParty.h"
#include <emmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

#include <chrono>

unsigned long rCounter;
NTLWrapper wrapper;
//one and zero constants
Wire *zeroWire, *oneWire;

using namespace std;


//encryption functionality
void encrypt(ZZ &result, int num, ZZ key, ZZ toEncrypt)
{
	result = wrapper.randomOracle(num);

	wrapper.multiplyZp(result, result, result);

	wrapper.invGen(result, result, key);

	wrapper.multiplyZp(result, result, toEncrypt);

}

void decrypt(ZZ &result, int num, ZZ key, ZZ toDecrypt)
{
	result = wrapper.randomOracle(num);
	wrapper.multiplyZp(result, result, result);
	wrapper.powerZp(result, result, key);
	wrapper.multiplyZp(result, result, toDecrypt);
}


void encryptWprecomputation(ZZ &result, ZZPrecomputeExp* generator, ZZ key, ZZ toEncrypt)
{
	result=generator->compInvGen(key);
	wrapper.multiplyZp(result, result, toEncrypt);
}

void decryptWprecomputation(ZZ &result, ZZPrecomputeExp* generator, ZZ key, ZZ toDecrypt)
{
#ifdef DLSE
 	result=generator->compExpDLSE(key);
#else
	result=generator->compExp(key);
#endif
	wrapper.multiplyZp(result, result, toDecrypt);
}



ZZ LoadSeedNew()
{
	rCounter++;

	ZZ ans;
	wrapper.randomKey(ans);


	return ans;
}

bool LoadBool()
{
	return wrapper.randomBool();//rand()%2;//0;//
}

void initializeRandomnessHom(char* key, int numOfParties)
{
    ZZ prime = GenGermainPrime_ZZ(KAPPA);
    cout << "Prime is: " << prime << endl;

    wrapper;
    wrapper.init(2*prime + 1,prime,2);

	srand(time(0));

#ifdef DLSE
    wrapper.smallExp=2;
    wrapper.exponent=DLSECONST;
    wrapper.powerZp(wrapper.smallExp,wrapper.smallExp,wrapper.exponent);
#endif

	rCounter = -1;
}


bool getValueHom(Wire wire)
{
      return wire.externalValue;
}

bool getValueHom(Wire *wire)
{
      return wire->externalValue;
}


bool getTrueValueHom(Wire wire)
{
      return wire.externalValue^wire.realLambda;
}

bool getTrueValueHom(Wire *wire)
{
      return wire->externalValue^wire->realLambda;
}

void loadInputsHom(char* filename, Circuit *cyc, int partyNum)
{
	char ch;
	int count = 0;
	bool val;
	FILE * f = fopen(filename, "r");

	if (f == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	while ((ch = fgetc(f)) != EOF && count<cyc->playerArray[partyNum].playerBitAmount)
	{
		if (ch == '1') val = 1; else val = 0;
		cyc->playerArray[partyNum].playerWires[count]->realValue = val;

		cyc->playerArray[partyNum].playerWires[count]->externalValue = cyc->playerArray[partyNum].playerWires[count]->realValue^cyc->playerArray[partyNum].playerWires[count]->realLambda;

		count++;
	}

	fclose(f);
}


Gate GateCreator(const unsigned int inputBit1, const unsigned int inputBit2, const unsigned int outputBit, TruthTable TTable, Wire * wireArray, unsigned int number)
{
	Gate g;
	g.gateNumber = number;

	g.input1 = &wireArray[inputBit1];
	g.input2 = &wireArray[inputBit2];
	g.output = &wireArray[outputBit];
	g.truthTable = TTable;

	g.flagNOMUL = !TTable.Y4;
	//NOTE: currently not dealing with "trivial" gates
	g.flagNOT = TTable.Y1;

	if (!g.flagNOMUL)
	{
		if (TTable.FF + TTable.TF + TTable.FT + TTable.TT == 1)
		{
			//Shifted AND gate, compute shift
			if (TTable.FF) g.sh = 3;
			else if (TTable.FT) g.sh = 2;
			else if (TTable.TF) g.sh = 1;
			//shift of negation
			if (wireArray[inputBit1].negation)
				g.sh ^= 2;
			if (wireArray[inputBit2].negation)
				g.sh ^= 1;
		}
		else
		{

			printf("ERROR: Unsupported gate.\n");
		}
	}
	else
	{
		if (TTable.Y2 + TTable.Y3 != 2)
		{
			printf("ERROR: Unsupported gate.\n");
		}
		else if (g.flagNOT)//XNOR gate
		{
			g.output->negation = !(g.input1->negation^g.input2->negation);
		}
		else//XOR gate
		{
			g.output->negation = g.input1->negation^g.input2->negation;
		}

	}

	return g;
}

inline unsigned int charToBooleanValue(char v){
	if (v == '1')
	{
		return true;
	}
	return false;
}


TruthTable createTruthTablefFromChars(char FF, char FT, char TF, char TT){
	TruthTable TrueT;
	TrueT.FF = charToBooleanValue(FF);
	TrueT.FT = charToBooleanValue(FT);
	TrueT.TF = charToBooleanValue(TF);
	TrueT.TT = charToBooleanValue(TT);
	TrueT.Y1 = TrueT.FF;
	TrueT.Y2 = TrueT.FF ^ TrueT.TF;
	TrueT.Y3 = TrueT.FF ^ TrueT.FT;
	TrueT.Y4 = TrueT.FF ^ TrueT.FT ^ TrueT.TF ^ TrueT.TT;
	return TrueT;
}

Circuit * readCircuitFromFile(char * path)
{
	unsigned int gateAmount = 0;
	unsigned int playerAmount = 0;
	unsigned int wiresAmount = 0;
	unsigned int lineCount = 1;
	unsigned int playerCounter = 0;
	unsigned int gateCounter = 0;



	unsigned int specialGatesAmount = 0;

	unsigned int i;

	unsigned int tempPlayerID;
	unsigned int tempAmountOfBits;
	unsigned int tempInput1;
	unsigned int tempInput2;
	unsigned int tempOutput;
	unsigned int tempStatus;

	char tempTT;
	char tempFT;
	char tempTF;
	char tempFF;

	char tempF;
	char tempT;
	TruthTable tempTruthTable;

	unsigned int gotOutputsBits = false;


	Circuit * cycleTR;
	char lineBuff[STRING_BUFFER_SIZE];
	char lineBuffCopy[STRING_BUFFER_SIZE];

	FILE * cycleFile = fopen(path, "r");

	if (cycleFile == NULL){
		printf("Error: Bad file path...");
		return NULL;
	}
	//read file, one line at a time
	while (fgets(lineBuff, STRING_BUFFER_SIZE, cycleFile)){
		//check for notes lines or empty lines. if found, continue.
		strcpy(lineBuffCopy, lineBuff);
		removeSpacesAndTabs(lineBuffCopy);
		if (lineBuffCopy[0] == 0 || lineBuffCopy[0] == '#' || lineBuffCopy[0] == '\n') continue;

		//will append in the first line and just one time.
		if (gateAmount == 0){
			if (sscanf(lineBuff, "%u %u %u", &gateAmount, &playerAmount, &wiresAmount) != 3){
				printf("Error: First line in the file had to be in format of '<amount of gates> <amount of players>'\ne.g '32123 3'");
				return NULL;
			}
			if (gateAmount <= 0){
				printf("Error: Amount of gates needs to be more than 0");
				return NULL;
			}
			// Init Circuit struct
			cycleTR = (Circuit*)malloc(sizeof(Circuit));
			cycleTR->gateArray = (Gate *)malloc(sizeof(Gate)*gateAmount);
			cycleTR->playerArray = (Player *)malloc(sizeof(Player)*playerAmount);

			cycleTR->allWires = (Wire *)malloc(sizeof(Wire)*wiresAmount);
			for (int i = 0; i < wiresAmount; i++)
			{
				cycleTR->allWires[i].number = i;
			}

			//cycleTR->amountOfGates = gateAmount;
			cycleTR->amountOfPlayers = playerAmount;
			cycleTR->amountOfWire = wiresAmount;



		}
		//players' input wires
		else if (playerCounter < playerAmount){
			if (sscanf(lineBuff, "P%u %u", &tempPlayerID, &tempAmountOfBits) == 2){
				//Init new player....
				cycleTR->playerArray[tempPlayerID].playerBitAmount = tempAmountOfBits;	//
				cycleTR->playerArray[tempPlayerID].playerBitArray = (unsigned int*)malloc(sizeof(unsigned int)*tempAmountOfBits);
				cycleTR->playerArray[tempPlayerID].playerWires = (Wire**)malloc(sizeof(Wire*)*tempAmountOfBits);
				for (i = 0; i < tempAmountOfBits; ++i) {
					if (!fgets(lineBuff, STRING_BUFFER_SIZE, cycleFile)) { printf("Error: in line %u bit serial expected... but the file is ended.", lineCount); return NULL; }
					lineCount++;

					if (sscanf(lineBuff, "%u", &cycleTR->playerArray[tempPlayerID].playerBitArray[i]) != 1) {
						printf("Error: in line %u expected for bit serial... ", lineCount); return NULL;
					}
					//pointer in player to input wire
					cycleTR->playerArray[tempPlayerID].playerWires[i] = &cycleTR->allWires[cycleTR->playerArray[tempPlayerID].playerBitArray[i]];

				}
				playerCounter++;
			}
			else{
				printf("Error: Player header expected. e.g P1 32");
				return NULL;
			}

		}
		//Output wires
		else if (!gotOutputsBits){
			gotOutputsBits = true;
			if (sscanf(lineBuff, "Out %u", &tempAmountOfBits) == 1){
				cycleTR->outputWires.playerBitAmount = tempAmountOfBits;
				cycleTR->outputWires.playerBitArray = (unsigned int*)malloc(sizeof(unsigned int)*tempAmountOfBits);
				cycleTR->outputWires.playerWires = (Wire**)malloc(sizeof(Wire*)*tempAmountOfBits);
				for (i = 0; i < cycleTR->outputWires.playerBitAmount; ++i) {
					if (!fgets(lineBuff, STRING_BUFFER_SIZE, cycleFile)) { printf("Error: in line %u bit serial expected... but the file is ended.", lineCount); return NULL; }
					lineCount++;
					if (sscanf(lineBuff, "%u", &cycleTR->outputWires.playerBitArray[i]) != 1) { printf("Error: in line %u expected for bit serial... ", lineCount); return NULL; }

					cycleTR->outputWires.playerWires[i] = &cycleTR->allWires[cycleTR->outputWires.playerBitArray[i]];
				}

			}
			else{
				printf("Error: Outputs header expected. e.g O 32");
				return NULL;
			}
		}
		//Gates
		else{
			if ((tempStatus = sscanf(lineBuff, "%u %u %u %c%c%c%c\n", &tempInput1, &tempInput2, &tempOutput, &tempFF, &tempFT, &tempTF, &tempTT)) >= 7){

				tempStatus = flagNone;
				//create truthtable of gate
				tempTruthTable = createTruthTablefFromChars(tempFF, tempFT, tempTF, tempTT);

				cycleTR->gateArray[gateCounter] = GateCreator(tempInput1, tempInput2, tempOutput, tempTruthTable,cycleTR->allWires,gateCounter);
				if (cycleTR->gateArray[gateCounter].flagNOMUL) specialGatesAmount++;

				gateCounter++;
			}

			else if ((tempStatus = sscanf(lineBuff, "%u %u %c%c\n", &tempInput1, &tempOutput, &tempF, &tempT)) == 4)
			{//not gates, make output wire negation of input wire. NOTE: not supported in this implementation.

				std::cout << "UNSUPPORTED GATE!!! " <<tempT<<tempF << std::endl;
			}
			else{
				printf("Error: Gate header expected.. format: <inputWire1> <inputWire2(optional)> <ouputWire> <truthTable>");
				return NULL;
			}
		}

		lineCount++;
	}

	if (gateCounter < gateAmount) {
		printf("Error: expected to %u gates, but got only %u...", gateAmount, gateCounter);
		return NULL;
	}

	cycleTR->amountOfGates = gateAmount;
	cycleTR->specialGates = specialGatesCollector(cycleTR->gateArray, gateAmount, specialGatesAmount);
	cycleTR->numOfXORGates = specialGatesAmount;
	cycleTR->numOfANDGates = gateAmount - specialGatesAmount;
	cycleTR->regularGates = regularGatesCollector(cycleTR->gateArray, gateAmount, cycleTR->numOfANDGates);


	fclose(cycleFile);

	return cycleTR;
}

char* truthTableToString(TruthTable TTB, char * charbuff){
	unsigned int count = 0;
	if (TTB.FF) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.FT) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.TF) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.TT) charbuff[count++] = '1'; else charbuff[count++] = '0';

	charbuff[count++] = ' ';
	if (TTB.Y1) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.Y2) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.Y3) charbuff[count++] = '1'; else charbuff[count++] = '0';
	if (TTB.Y4) charbuff[count++] = '1'; else charbuff[count++] = '0';

	charbuff[count++] = '\0';
	return charbuff;
}

void printCircuit(const Circuit * c)
{
	char trueTBuffer[30];
	char * flagsFriendlyNames[] = { "", dispXor, dispXnor };
	int p, i;
	if (c == NULL) {
		printf("Error: got NULL...\n");
	}
	else{
		printf("Gates amount: %u, players: %u\n", c->amountOfGates, c->amountOfPlayers);
		for (p = 0; p < c->amountOfPlayers; p++){
			printf("P%d %u\n", p, c->playerArray[p].playerBitAmount);
			for (i = 0; i < c->playerArray[p].playerBitAmount; ++i) {
				printf("%u\n", c->playerArray[p].playerBitArray[i]);
			}
		}
		printf("Out %u\n", c->outputWires.playerBitAmount);
		for (i = 0; i < c->outputWires.playerBitAmount; ++i) {
			printf("%u\n", c->outputWires.playerBitArray[i]);
		}
		for (i = 0; i<c->amountOfGates; i++){
			printf("%u %u %u %s %d\n", c->gateArray[i].input1->number, c->gateArray[i].input2->number, c->gateArray[i].output->number,
				truthTableToString(c->gateArray[i].truthTable, trueTBuffer), c->gateArray[i].flagNOMUL);
		}
	}
}



void removeSpacesAndTabs(char* source)
{
	char* i = source;
	char* j = source;
	while (*j != 0)
	{
		*i = *j++;
		if (*i != ' ' && *i != '\t')
			i++;
	}
	*i = 0;
}



Gate ** specialGatesCollector(Gate * GatesArray, const unsigned int arraySize, const unsigned int specialGatesAmount){
	unsigned int i;
	unsigned int count = 0;
	Gate ** sarray = (Gate**)malloc(sizeof(Gate*) * specialGatesAmount);
	for (i = 0; i < arraySize; ++i) {
		if (GatesArray[i].flagNOMUL){
			sarray[count++] = &GatesArray[i];
		}
		if (count>specialGatesAmount){
			printf("Error: specialGatesCollector - count>specialGatesAmount");
			return NULL;
		}
	}
	return sarray;
}

Gate ** regularGatesCollector(Gate * GatesArray, const unsigned int arraySize, const unsigned int regularGatesAmount){
	unsigned int i;
	unsigned int count = 0;
	Gate ** sarray = (Gate**)malloc(sizeof(Gate*) * regularGatesAmount);
	for (i = 0; i < arraySize; ++i) {
		if (!GatesArray[i].flagNOMUL){
			sarray[count++] = &GatesArray[i];
		}
		if (count>regularGatesAmount){
			printf("Error: regularGatesCollector - count>regularGatesAmount");
			return NULL;
		}
	}
	return sarray;
}

//TODO: write destructors (not needed for our purposes)

void freeWire(Wire* w)
{
  //TODO
}

void freeGate(Gate* gate)
{
      //TODO
}

void freePlayer(Player p)
{
	//TODO?
	delete[] p.playerBitArray;
	delete[] p.playerWires;
}

void freeCircuitPartial(Circuit * c)
{
	//TODO?
	if (!c)
	{
		printf("Error: expected circuit, but got NULL...");
		return;
	}
	for (int i = 0; i < c->numOfANDGates; i++)
	{
		freeGate(c->regularGates[i]);
	}
}

void freeCircle(Circuit * c)
{
	//TODO
}

int getrCounter()
{
	return rCounter;
}

/*
* secCompMultiParty.c
*
*  Created on: Mar 31, 2015
*      Author: froike (Roi Inbar)
*      Modified: Aner Ben-Efraim
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "secCompMultiParty.h"
#include <stdint.h>

#include <chrono>

//for randomness
__m128i pseudoRandomString[RANDOM_COMPUTE];
__m128i tempSecComp[RANDOM_COMPUTE];
AES_KEY_TED aes_key;

unsigned long rCounter;
//one and zero constants
Wire *zeroWire, *oneWire;

using namespace std;


uint32_t LoadSeedNew()
{
	rCounter++;
	if (rCounter % RANDOM_COMPUTE == 0)
	{
		for (int i = 0; i < RANDOM_COMPUTE; i++)
			tempSecComp[i] = _mm_set1_epi32(rCounter+i);
		AES_ecb_encrypt_chunk_in_out(tempSecComp, pseudoRandomString, RANDOM_COMPUTE, &aes_key);
	}
	return _mm_extract_epi32(pseudoRandomString[rCounter%RANDOM_COMPUTE],0);
}

bool LoadBool()
{
	return LoadSeedNew()%2;
}

void initializeRandomnessHom(char* key, int numOfParties)
{
	AES_set_encrypt_key((unsigned char*)key, 256, &aes_key);

	rCounter = -1;
}

////returns the value of the wire
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

//load the inputs
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

int setFanOut(Circuit* circuit)
{
	int maxfanout=0;
	Gate* gate;
	Wire* wire;
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
	return numOfGenerators;
}


Gate GateCreator(const unsigned int inputBit1, const unsigned int inputBit2, const unsigned int outputBit, TruthTable TTable, Wire * wireArray, unsigned int number)
{
	Gate g;
	g.gateNumber = number;

	g.input1 = &wireArray[inputBit1];
	g.input2 = &wireArray[inputBit2];
	g.output = &wireArray[outputBit];
	g.truthTable = TTable;

	g.flagNOMUL = !TTable.Y4;//Check if it's a XOR/XNOR
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


	Circuit * circuitTR;
	char lineBuff[STRING_BUFFER_SIZE];
	char lineBuffCopy[STRING_BUFFER_SIZE];

	FILE * circuitFile = fopen(path, "r");

	if (circuitFile == NULL){
		printf("Error: Bad file path...");
		return NULL;
	}
	//read file, one line at a time
	while (fgets(lineBuff, STRING_BUFFER_SIZE, circuitFile)){
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
			circuitTR = (Circuit*)malloc(sizeof(Circuit));
			circuitTR->gateArray = (Gate *)malloc(sizeof(Gate)*gateAmount);
			circuitTR->playerArray = (pPlayer *)malloc(sizeof(pPlayer)*playerAmount);

			circuitTR->allWires = (Wire *)malloc(sizeof(Wire)*wiresAmount);//(Irrelevant) +2 for one and zero
			for (int i = 0; i < wiresAmount; i++)
			{
				circuitTR->allWires[i].number = i;

			}

			//circuitTR->amountOfGates = gateAmount;
			circuitTR->amountOfPlayers = playerAmount;
			circuitTR->amountOfWire = wiresAmount;



		}
		//players' input wires
		else if (playerCounter < playerAmount){
			if (sscanf(lineBuff, "P%u %u", &tempPlayerID, &tempAmountOfBits) == 2){
				//Init new player....
				circuitTR->playerArray[tempPlayerID].playerBitAmount = tempAmountOfBits;	//
				circuitTR->playerArray[tempPlayerID].playerBitArray = (unsigned int*)malloc(sizeof(unsigned int)*tempAmountOfBits);
				circuitTR->playerArray[tempPlayerID].playerWires = (Wire**)malloc(sizeof(Wire*)*tempAmountOfBits);
				for (i = 0; i < tempAmountOfBits; ++i) {
					if (!fgets(lineBuff, STRING_BUFFER_SIZE, circuitFile)) { printf("Error: in line %u bit serial expected... but the file is ended.", lineCount); return NULL; }
					lineCount++;

					if (sscanf(lineBuff, "%u", &circuitTR->playerArray[tempPlayerID].playerBitArray[i]) != 1) {
						printf("Error: in line %u expected for bit serial... ", lineCount); return NULL;
					}
					//pointer in player to input wire
					circuitTR->playerArray[tempPlayerID].playerWires[i] = &circuitTR->allWires[circuitTR->playerArray[tempPlayerID].playerBitArray[i]];

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
				circuitTR->outputWires.playerBitAmount = tempAmountOfBits;
				circuitTR->outputWires.playerBitArray = (unsigned int*)malloc(sizeof(unsigned int)*tempAmountOfBits);
				circuitTR->outputWires.playerWires = (Wire**)malloc(sizeof(Wire*)*tempAmountOfBits);
				for (i = 0; i < circuitTR->outputWires.playerBitAmount; ++i) {
					if (!fgets(lineBuff, STRING_BUFFER_SIZE, circuitFile)) { printf("Error: in line %u bit serial expected... but the file is ended.", lineCount); return NULL; }
					lineCount++;
					if (sscanf(lineBuff, "%u", &circuitTR->outputWires.playerBitArray[i]) != 1) { printf("Error: in line %u expected for bit serial... ", lineCount); return NULL; }

					circuitTR->outputWires.playerWires[i] = &circuitTR->allWires[circuitTR->outputWires.playerBitArray[i]];
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

				circuitTR->gateArray[gateCounter] = GateCreator(tempInput1, tempInput2, tempOutput, tempTruthTable,circuitTR->allWires,gateCounter);
				if (circuitTR->gateArray[gateCounter].flagNOMUL) specialGatesAmount++;

				gateCounter++;
			}

			else if ((tempStatus = sscanf(lineBuff, "%u %u %c%c\n", &tempInput1, &tempOutput, &tempF, &tempT)) == 4)
			{//not gates, make output wire negation of input wire. NOTE: not dealing with other gates (buffer, 1/0) currently.

				//TODO (not dealt with in this implementation)

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

	circuitTR->amountOfGates = gateAmount;

	fclose(circuitFile);

	circuitTR->numOfInputWires = 0;
	for (int p = 0; p < circuitTR->amountOfPlayers; p++)
	{
		circuitTR->numOfInputWires += circuitTR->playerArray[p].playerBitAmount;
	}

	return circuitTR;
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
	string flagsFriendlyNames[] = { "", dispXor, dispXnor };
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


//TODO:implement destructors (not needed for our purposes)

void freeWire(Wire* w)
{
  //TODO
}

void freeGate(Gate* gate)
{
      //TODO
}

void freePlayer(pPlayer p)
{
	//TODO?
	delete[] p.playerBitArray;
	delete[] p.playerWires;
}

void freeCircuitPartial(Circuit * c)
{
	//TODO

}

void freeCircuit(Circuit * c)
{
	//TODO
}

int getrCounter()
{
	return rCounter;
}

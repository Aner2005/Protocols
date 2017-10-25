#include "BMR_BGW_Latincrypt.h"

int numOfPartiesLC;


void sendPrivateKey(int party)
{

    //send
    communicationSenders[party]->sendMsg(&privateKeysSent[party],sizeof(__m128i));
}
void receivePrivateKey(int party)
{

    __m128i* temp=static_cast<__m128i *>(_aligned_malloc(sizeof(__m128i), 16));

    communicationReceivers[party]->reciveMsg(temp,sizeof(__m128i));

    privateKeysReceived[party]=temp[0];

    _aligned_free(temp);

}

void initializePrivateKeys()
{
    privateKeysSent=static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
    for (int p=0;p<numOfParties;p++)
        privateKeysSent[p]=RAND;
    privateKeysReceived=static_cast<__m128i *>(_aligned_malloc(numOfParties * sizeof(__m128i), 16));
    //send and receive privateKeys
	thread *threads = new thread[numOfParties * 2];
	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{
			threads[i * 2] = thread(sendPrivateKey ,i);//send private key to player i
			threads[i * 2 + 1] = thread(receivePrivateKey , i);//receive private key from player i
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

void donothingTest()
{
    cout<<"hey!"<<endl;
}


void computeGatesHME()
{

    if (majType<20 && majType != 2)
        numOfPartiesLCNew=numOfParties/2+1;
    else
        numOfPartiesLCNew=numOfParties/3+1;

    //Determine length of superseed!
    numOfPartiesLC=numOfParties;

    if (majType==12 || majType==13 || majType==14)
    {
        if (numOfParties%2 != 1)
        {
            cout <<"Number of parties must odd for this version!"<<endl;
            exit(1);
        }
        numOfPartiesLC=numOfParties/2+1;//shorten superseed length by half
    }
    else if (majType==22 || majType==23 || majType==24)
    {
        if (numOfParties%3 != 1)
        {
            cout <<"Number of parties must 1 mod 3 this version!"<<endl;
            exit(1);
        }
        numOfPartiesLC=numOfParties/3+1;//shorten superseed length by 2/3
    }



    bigMemoryAllocation = static_cast<__m128i *>(_aligned_malloc((4 * cyc->numOfANDGates*numOfPartiesLC+numOfOutputWires) * sizeof(__m128i), 16));

    __m128i seed1, seed2;
    Gate* gate;
    if (partyNum<numOfPartiesLC)
    {
        for (int g = 0; g < cyc->numOfANDGates; g++)
        {
            gate = cyc->regularGates[g];

            seed1 = gate->input1->seed;
            seed2 = gate->input2->seed;


            for (int i = 0; i < 4; i++)
                gate->G[i] = &bigMemoryAllocation[(4*g+i)*numOfPartiesLC];


            PSEUDO_RANDOM_FUNCTION(seed1, seed2, gate->gateNumber, numOfPartiesLC, gate->G[0]);//in1,in2
            PSEUDO_RANDOM_FUNCTION(seed1, _mm_xor_si128(seed2, R), gate->gateNumber, numOfPartiesLC, gate->G[1]);//in1,in2^R
            PSEUDO_RANDOM_FUNCTION(_mm_xor_si128(seed1, R), seed2, gate->gateNumber, numOfPartiesLC, gate->G[2]);//in1^R,in2
            PSEUDO_RANDOM_FUNCTION(_mm_xor_si128(seed1, R), _mm_xor_si128(seed2, R), gate->gateNumber, numOfPartiesLC, gate->G[3]);//in1^r,in2^R

            gate->G[0][partyNum] = _mm_xor_si128(gate->G[0][partyNum], gate->output->seed);
            gate->G[1][partyNum] = _mm_xor_si128(gate->G[1][partyNum], gate->output->seed);
            gate->G[2][partyNum] = _mm_xor_si128(gate->G[2][partyNum], gate->output->seed);
            gate->G[3][partyNum] = _mm_xor_si128(gate->G[3][partyNum], gate->output->seed);
        }
    }
    else
    {
        for (int g = 0; g < cyc->numOfANDGates; g++)
        {
            gate = cyc->regularGates[g];


            for (int i = 0; i < 4; i++)
                gate->G[i] = &bigMemoryAllocation[(4*g+i)*numOfPartiesLC];

            for (int p=0;p<numOfPartiesLC;p++)
            {
                gate->G[0][p] = ZERO;
                gate->G[1][p] = ZERO;
                gate->G[2][p] = ZERO;
                gate->G[3][p] = ZERO;
            }

        }
    }

}

void exchangeSeedsHME()
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
        if (partyNum < numOfPartiesLC)//only contributing parties need to send their input seeds
		    sendSeed(toSend,0);
	}
	else
	{
		thread *threads = new thread[numOfPartiesLC];
		for (int i = 0; i < numOfPartiesLC; i++)
		{
			if (i != partyNum)
			{
				threads[i] = thread(receiveSeed, i);//receive inputs from player i
			}
		}
		for (int i = 0; i < numOfPartiesLC; i++)
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

void secretShareHME()
{


    if (majType != 13 && majType != 14 && majType != 23 && majType != 24)
        totalSharesNumber = (4 * numOfPartiesLC)*cyc->numOfANDGates + (cyc->numOfANDGates + numOfInputWires) + 1;
    else
        totalSharesNumber = cyc->numOfANDGates + numOfInputWires + 1;

	shares = static_cast<__m128i *>(_aligned_malloc(totalSharesNumber*sizeof(__m128i), 16));
	memset(shares, 0, totalSharesNumber*sizeof(*shares));
	//alocate memory for storing shares of R
	RShare= static_cast<__m128i *>(_aligned_malloc(numOfParties*sizeof(__m128i), 16));//for saving shares of R^i


    generatePolynomialsLatinCryptHME();

	int sizeToSend, sizeSendBig, sizeSendSmall;


		sizeSendBig=totalSharesNumber;
		sizeSendSmall=(cyc->numOfANDGates + numOfInputWires) + 1;
		if (partyNum<numOfPartiesLCNew)
			sizeToSend=sizeSendBig;
		else
			sizeToSend=sizeSendSmall;

	thread *threads = new thread[numOfParties * 2];

	for (int i = 0; i < numOfParties; i++)
	{
		if (i != partyNum)
		{

			threads[i * 2] = thread(sendSharesHME, i,sizeToSend);//generate and send shares to player i.
            if (i<numOfPartiesLCNew)
			    threads[i * 2 + 1] = thread(receiveSharesHME, i,sizeSendBig);//receive shares from player i and sum (XOR) them. Also saves share of R^i
            else
                threads[i * 2 + 1] = thread(receiveSharesHME, i,sizeSendSmall);
		}
		else
			threads[i * 2] = thread(addLocalSharesHME);//add (XOR) the local shares
	}


	for (int i = 0; i < 2 * numOfParties; i++)
	{
		if (i != (2 * partyNum + 1))//i != 2 * partyNum &&
		{
			threads[i].join();//wait for all threads to finish
		}
	}
	//locate the shares in the appropriate places

	int count = 1;//Start counting from 1 because 0 is shares of R.
	for (int p = 0; p < numOfParties; p++)
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			cyc->playerArray[p].playerWires[i]->lambdaShare = shares[count];
			count++;
		}

	for (int g = 0; g < cyc->numOfANDGates; g++)
	{
		cyc->regularGates[g]->output->lambdaShare = shares[count];
		count++;
	}

    if (majType != 13 && majType != 14 && majType != 23 && majType != 24)
    {
    	for (int g = 0; g < cyc->numOfANDGates; g++)
    		for (int i = 0; i < 4; i++)
    			for (int p = 0; p < numOfPartiesLC; p++)
    			{
    				cyc->regularGates[g]->G[i][p] = shares[count];
    				count++;
    			}
    }
    else // zero-sharing from PRG
    {

        for (int g = 0; g < cyc->numOfANDGates; g++)
    		for (int i = 0; i < 4; i++)
    			for (int p = 0; p < numOfPartiesLC; p++)
    			{
                    for (int party = 0; party < numOfParties; party++)
                    {
                        if (party!=partyNum)
                            cyc->regularGates[g]->G[i][p]=ADD(cyc->regularGates[g]->G[i][p],ZEROSHARE(party,g,i,p));
                    }

    			}
    }
	//free memory
	_aligned_free(allSecrets);

	_aligned_free(allCoefficients);

	_aligned_free(shares);

	delete[] threads;


	for (int g = 0; g < cyc->numOfXORGates; g++)
		cyc->specialGates[g]->output->lambdaShare = _mm_xor_si128(cyc->specialGates[g]->input1->lambdaShare, cyc->specialGates[g]->input2->lambdaShare);


}

//Parallel sending
void newSendGateBGW_HME(__m128i* toSend, int player)
{
	int totalSendSize = ((4 * cyc->numOfANDGates*numOfPartiesLC) + numOfOutputWires)*sizeof(__m128i);

	communicationSenders[player]->sendMsg(toSend, totalSendSize);
}

void newSendGateBGW_HME_TEST(__m128i* toSend, int player)
{
	int totalSendSize = sizeof(__m128) * 4 * cyc->numOfANDGates *numOfPartiesLC+numOfOutputWires;

	communicationSenders[player]->sendMsg(toSend, totalSendSize);
}
void newReceiveGateLatinCrypt_TEST(int party, __m128i* summationVector, int totalLength)
{

	__m128i* tempArr = static_cast<__m128i *>(_aligned_malloc(totalLength, 16));//assign space;
	communicationReceivers[party]->reciveMsg(tempArr, totalLength);

    mtxVal.lock();
	XORvectors(summationVector, tempArr, summationVector, 4 * cyc->numOfANDGates *numOfPartiesLC);
	for (int o = 0; o < numOfOutputWires; o++)
	{
		((bool*)summationVector)[4 * cyc->numOfANDGates *numOfPartiesLC*sizeof(__m128i) + o] ^= ((bool*)tempArr)[4 * cyc->numOfANDGates *numOfPartiesLC*sizeof(__m128i) + o];
	}
    mtxVal.unlock();
	//free space
	_aligned_free(tempArr);
}
//Parallel sending
void newReceiveGateBGW_HME(int player)
{
	int totalReceiveSize = ((4 * cyc->numOfANDGates*numOfPartiesLC) + numOfOutputWires)*sizeof(__m128i);
	playerSeeds[player] = static_cast<__m128i *>(_aligned_malloc(totalReceiveSize, 16));
	communicationReceivers[player]->reciveMsg(playerSeeds[player], totalReceiveSize);
}


#ifdef MULTTHREADNUM
void sumWithThreads(int tNum, int totalThreads)
{
    int start = (tNum* cyc->numOfANDGates) / totalThreads;
	int end = ((tNum + 1)* cyc->numOfANDGates) / totalThreads;
    for (int g = start; g < end; g++)//for every AND gate
    {
        for (int p = 0; p < numOfPartiesLC; p++)//for all the superseed
        {
            for (int party = 0; party < numOfParties; party++)//for shares of every party
            {
                if (party!=partyNum)//local share should already be in cyc->regularGates[g]->G[row][p]
                {

                    cyc->regularGates[g]->G[0][p] = ADD(cyc->regularGates[g]->G[0][p],playerSeeds[party][4 * g* numOfPartiesLC + p]);
                    cyc->regularGates[g]->G[1][p] = ADD(cyc->regularGates[g]->G[1][p],playerSeeds[party][(4 * g + 1)*numOfPartiesLC + p]);
                    cyc->regularGates[g]->G[2][p] = ADD(cyc->regularGates[g]->G[2][p],playerSeeds[party][(4 * g + 2)*numOfPartiesLC + p]);
                    cyc->regularGates[g]->G[3][p] = ADD(cyc->regularGates[g]->G[3][p],playerSeeds[party][(4 * g + 3)*numOfPartiesLC + p]);
                }
            }
        }
    }
}

void allocateWithThreads(int tNum, int totalThreads, __m128i* toSend)
{
    int start = (tNum* cyc->numOfANDGates) / totalThreads;
	int end = ((tNum + 1)* cyc->numOfANDGates) / totalThreads;
    for (int g = start; g < end; g++)
    {
        for (int p = 0; p < numOfPartiesLC; p++)
        {
            cyc->regularGates[g]->G[0][p] = toSend[(4 * g)*numOfPartiesLC + p];
            cyc->regularGates[g]->G[1][p] = toSend[(4 * g + 1)*numOfPartiesLC + p];
            cyc->regularGates[g]->G[2][p] = toSend[(4 * g + 2)*numOfPartiesLC + p];
            cyc->regularGates[g]->G[3][p] = toSend[(4 * g + 3)*numOfPartiesLC + p];
        }
    }
}

#endif //MULTTHREADNUM

void exchangeGatesBGWLatinCryptHME()
{

    int reconDeg = degBig;

    int exchangingParties = numOfParties;


        int totalSendSize = ((4 * cyc->numOfANDGates*numOfPartiesLC)+numOfOutputWires)*sizeof(__m128i);
    __m128i* toSend;

    toSend = bigMemoryAllocation;


    for (int o = 0; o < numOfOutputWires; o++)
    {
        toSend[4 * cyc->numOfANDGates *numOfPartiesLC + o] = cyc->outputWires.playerWires[o]->lambdaShare;
    }


        if (partyNum!=0)//not evaluating party
    {
        newSendGateBGW_HME(toSend,0);//send inputs to player 0
    }
    else//party 0 - evaluating party
    {

        playerSeeds = new __m128i*[exchangingParties];
        thread *threads = new thread[exchangingParties];
        for (int i = 0; i < exchangingParties; i++)
        {
            if (i != partyNum)
            {
                threads[i] = thread(newReceiveGateBGW_HME, i);//receive inputs from player i
            }
        }
        for (int i = 0; i < exchangingParties; i++)
        {
            if (i !=  partyNum )
            {
                threads[i].join();//wait for all threads to finish
            }
        }


#ifndef MULTTHREADNUM

        for (int g = 0; g < cyc->numOfANDGates; g++)
        {
            for (int p = 0; p < numOfPartiesLC; p++)
            {
                for (int party = 0; party < exchangingParties; party++)
                {
                    if (party!=partyNum)
                    {
                        cyc->regularGates[g]->G[0][p] = ADD(cyc->regularGates[g]->G[0][p],playerSeeds[party][4 * g* numOfPartiesLC + p]);
                        cyc->regularGates[g]->G[1][p] = ADD(cyc->regularGates[g]->G[1][p],playerSeeds[party][(4 * g + 1)*numOfPartiesLC + p]);
                        cyc->regularGates[g]->G[2][p] = ADD(cyc->regularGates[g]->G[2][p],playerSeeds[party][(4 * g + 2)*numOfPartiesLC + p]);
                        cyc->regularGates[g]->G[3][p] = ADD(cyc->regularGates[g]->G[3][p],playerSeeds[party][(4 * g + 3)*numOfPartiesLC + p]);
                    }
                }
            }
        }
#else//MULTTHREADNUM defined
        int threadNum = min(MULTTHREADNUM *numOfParties,16);
        thread *threadss = new thread[threadNum];
        for (int t = 0; t < threadNum; t++)
            threadss[t] = thread(sumWithThreads, t, threadNum);//add (XOR) the local shares

        for (int t = 0; t < threadNum; t++)
            threadss[t].join();//wait for all threads to finish
        delete[] threadss;
#endif //MULTTHREADNUM

        int totalSharesSize = (exchangingParties)*sizeof(__m128i);
        __m128i* shares0 = static_cast<__m128i *>(_aligned_malloc(totalSharesSize, 16));

        int startIndex = 4 * cyc->numOfANDGates *numOfPartiesLC;
        for (int o = 0; o < numOfOutputWires; o++)
        {
            for (int party = 0; party < exchangingParties; party++)
            {
                if (party != partyNum)
                {
                    shares0[party] = playerSeeds[party][ startIndex+ o];
                }
                else
                {
                    shares0[party] = toSend[startIndex + o];
                }
            }

            __m128i recon=reconstruct(shares0, reconDeg);

            if (EQ(recon, ONE))
            {
                cyc->outputWires.playerWires[o]->lambda = true;
            }
            else
            {
                cyc->outputWires.playerWires[o]->lambda = false;
            }
        }

        //Free memory
        _aligned_free(shares0);
        delete[] threads;
        for (int p = 0; p < exchangingParties; p++)
            if (p != partyNum)
                _aligned_free(playerSeeds[p]);
        delete[] playerSeeds;
    }
}

void exchangeGatesBGWLatinCryptHMElognRounds()
{

    int reconDeg = degBig;

    int exchangingParties = numOfParties;

    int outputSize = numOfOutputWires;//I think this should be padded to a multiple of 128
	int totalSendSize = sizeof(__m128) * 4 * cyc->numOfANDGates *numOfPartiesLC+outputSize;

    __m128i* toSend;

    toSend = bigMemoryAllocation;

    for (int o = 0; o < numOfOutputWires; o++)
		((bool*)bigMemoryAllocation)[4 * cyc->numOfANDGates *numOfPartiesLC*sizeof(__m128i) + o] = cyc->outputWires.playerWires[o]->lambda;




    int numOfPartiesToReceive=0;
    int* partiesToReceive=new int[numOfParties];
    int partyToSend;

    int remainingParties=numOfParties;
    int halfRemainingParties=(remainingParties+1)/2;
    while(partyNum+halfRemainingParties < remainingParties)
    {

            partiesToReceive[numOfPartiesToReceive]=partyNum+halfRemainingParties;
            numOfPartiesToReceive++;

        remainingParties=halfRemainingParties;
        halfRemainingParties=(remainingParties+1)/2;
    }
    if (partyNum!=0)
    {
        while(partyNum<halfRemainingParties)
        {
            remainingParties=halfRemainingParties;
            halfRemainingParties=(remainingParties+1)/2;
        }
        partyToSend = partyNum - halfRemainingParties;
    }

    thread *threads= new thread[numOfPartiesToReceive];
    for (int t = 0; t < numOfPartiesToReceive; t++)
    {
        threads[t]= thread(newReceiveGateLatinCrypt_TEST, partiesToReceive[t],toSend,totalSendSize);
    }


    for (int t = 0; t < numOfPartiesToReceive; t++)
    {
        threads[t].join();//wait for all threads to finish
    }
    delete[] threads;
    delete[] partiesToReceive;

    if (partyNum!=0)
    {

        newSendGateBGW_HME_TEST(toSend, partyToSend);
    }

	if (partyNum==0)
	{

#ifndef MULTTHREADNUM
				for (int g = 0; g < cyc->numOfANDGates; g++)
				{
					for (int p = 0; p < numOfPartiesLC; p++)
					{
						cyc->regularGates[g]->G[0][p] = toSend[(4 * g)*numOfPartiesLC + p];
						cyc->regularGates[g]->G[1][p] = toSend[(4 * g + 1)*numOfPartiesLC + p];
						cyc->regularGates[g]->G[2][p] = toSend[(4 * g + 2)*numOfPartiesLC + p];
						cyc->regularGates[g]->G[3][p] = toSend[(4 * g + 3)*numOfPartiesLC + p];
					}
				}

#else//MULTTHREADNUM defined
        int threadNum = min(MULTTHREADNUM *numOfParties,16);
        thread *threadss = new thread[threadNum];
        for (int t = 0; t < threadNum; t++)
            threadss[t] = thread(allocateWithThreads, t, threadNum, toSend);

        for (int t = 0; t < threadNum; t++)
            threadss[t].join();//wait for all threads to finish
        delete[] threadss;
#endif //MULTTHREADNUM
                for (int o = 0; o < numOfOutputWires;o++)
                    cyc->outputWires.playerWires[o]->lambda = ((bool*) toSend)[(sizeof(__m128i)/sizeof(bool))* 4 * cyc->numOfANDGates *numOfPartiesLC + o];

	}
}

#ifdef MULTTHREADNUM

void mulRBGWwShareConversionwThreads(int tNum, int totalThreads)
{

    int start = (tNum*cyc->numOfANDGates) / totalThreads;
	int end = ((tNum + 1)*cyc->numOfANDGates) / totalThreads;
	__m128i reconstructionConstant=baseRecon[partyNum];

	__m128i tmpShare0;//share of (mulLambdas+outLambda)
	__m128i tmpShare1;//share of mulLambdas+inLambda1+outLambda
	__m128i tmpShare2;//share of mulLambdas+inLambda2+outLambda
	__m128i tmpShare3;//share of mulLambdas+inLambda1+inLambda2+outLambda

	__m128i tmpMultShare0;//share of (mulLambdas+outLambda) * R^i
	__m128i tmpMultShare1;//share of mulLambdas+inLambda1+outLambda * R^i
	__m128i tmpMultShare2;//share of mulLambdas+inLambda2+outLambda * R^i
	__m128i tmpMultShare3;//share of mulLambdas+inLambda1+inLambda2+outLambda * R^i

		Gate* gate;
		int shift;
		for (int g = start; g < end; g++)
		{
			gate = cyc->regularGates[g];
			shift = gate->sh;
			tmpShare0 = ADD(gate->mulLambdaShare, gate->output->lambdaShare);
			tmpShare1 = ADD(tmpShare0, gate->input1->lambdaShare);
			tmpShare2 = ADD(tmpShare0, gate->input2->lambdaShare);
			tmpShare3 = ADD(ADD(tmpShare1, gate->input2->lambdaShare),ONE);

			for (int p = 0; p < numOfPartiesLC; p++)
			{
				//multiply lambda shares with R
				tmpMultShare0 = MUL(tmpShare0, RShare[p]);
				tmpMultShare1 = MUL(tmpShare1, RShare[p]);
				tmpMultShare2 = MUL(tmpShare2, RShare[p]);
				tmpMultShare3 = MUL(tmpShare3, RShare[p]);

				//do share conversion to additive secret-sharing (multiply by reconstruction constant)
				tmpMultShare0 = MUL(tmpMultShare0,reconstructionConstant);
				tmpMultShare1 = MUL(tmpMultShare1,reconstructionConstant);
				tmpMultShare2 = MUL(tmpMultShare2,reconstructionConstant);
				tmpMultShare3 = MUL(tmpMultShare3,reconstructionConstant);

				//save in correct locations
				gate->G[0 ^ shift][p] = ADD(gate->G[0 ^ shift][p], tmpMultShare0);
				gate->G[1 ^ shift][p] = ADD(gate->G[1 ^ shift][p], tmpMultShare1);
				gate->G[2 ^ shift][p] = ADD(gate->G[2 ^ shift][p], tmpMultShare2);
				gate->G[3 ^ shift][p] = ADD(gate->G[3 ^ shift][p], tmpMultShare3);

			}
		}
}

#endif//MULTTHREADNUM


void mulRBGWwShareConversionHME()
{

#ifndef MULTTHREADNUM
    __m128i reconstructionConstant=baseRecon[partyNum];

    __m128i tmpShare0;//share of (mulLambdas+outLambda)
    __m128i tmpShare1;//share of mulLambdas+inLambda1+outLambda
    __m128i tmpShare2;//share of mulLambdas+inLambda2+outLambda
    __m128i tmpShare3;//share of mulLambdas+inLambda1+inLambda2+outLambda

    __m128i tmpMultShare0;//share of (mulLambdas+outLambda) * R^i
    __m128i tmpMultShare1;//share of mulLambdas+inLambda1+outLambda * R^i
    __m128i tmpMultShare2;//share of mulLambdas+inLambda2+outLambda * R^i
    __m128i tmpMultShare3;//share of mulLambdas+inLambda1+inLambda2+outLambda * R^i

        Gate* gate;
        int shift;
        for (int g = 0; g < cyc->numOfANDGates; g++)
        {
            gate = cyc->regularGates[g];
            shift = gate->sh;
            tmpShare0 = ADD(gate->mulLambdaShare, gate->output->lambdaShare);
            tmpShare1 = ADD(tmpShare0, gate->input1->lambdaShare);
            tmpShare2 = ADD(tmpShare0, gate->input2->lambdaShare);
            tmpShare3 = ADD(ADD(tmpShare1, gate->input2->lambdaShare),ONE);

            for (int p = 0; p < numOfPartiesLC; p++)//Add share of R*tmpShare to the share of the PRG+S
            {
                //multiply lambda shares with R
                tmpMultShare0 = MUL(tmpShare0, RShare[p]);
                tmpMultShare1 = MUL(tmpShare1, RShare[p]);
                tmpMultShare2 = MUL(tmpShare2, RShare[p]);
                tmpMultShare3 = MUL(tmpShare3, RShare[p]);

                //do share conversion to additive secret-sharing (multiply by reconstruction constant)
                tmpMultShare0 = MUL(tmpMultShare0,reconstructionConstant);
                tmpMultShare1 = MUL(tmpMultShare1,reconstructionConstant);
                tmpMultShare2 = MUL(tmpMultShare2,reconstructionConstant);
                tmpMultShare3 = MUL(tmpMultShare3,reconstructionConstant);

                //save in correct locations
                gate->G[0 ^ shift][p] = ADD(gate->G[0 ^ shift][p], tmpMultShare0);
                gate->G[1 ^ shift][p] = ADD(gate->G[1 ^ shift][p], tmpMultShare1);
                gate->G[2 ^ shift][p] = ADD(gate->G[2 ^ shift][p], tmpMultShare2);
                gate->G[3 ^ shift][p] = ADD(gate->G[3 ^ shift][p], tmpMultShare3);

            }
        }

        _aligned_free(RShare);//shares of R not needed anymore

        #else //MULTTHREADNUM defined
        	int threadNum = min(MULTTHREADNUM *numOfParties,16);
        	thread *threads = new thread[threadNum];
        	for (int i = 0; i < threadNum; i++)
        		threads[i] = thread(mulRBGWwShareConversionwThreads, i, threadNum);//multiply and convert

        	for (int i = 0; i < threadNum; i++)
        		threads[i].join();//wait for all threads to finish
        	delete[] threads;
        #endif //MULTTHREADNUM
}


void sendSharesHME(int party, int sizeToSend)
{

	__m128i* sendShares = static_cast<__m128i *>(_aligned_malloc(totalSharesNumber*sizeof(__m128i), 16));


	computeSharesLatinCryptHME(party,sendShares);


	communicationSenders[party]->sendMsg(sendShares, sizeToSend*sizeof(__m128i));

	_aligned_free(sendShares);
}

void receiveSharesHME(int party, int sizeToReceive)
{

	__m128i* receivedShares = static_cast<__m128i *>(_aligned_malloc(totalSharesNumber*sizeof(__m128i), 16));
	//receive shares

	communicationReceivers[party]->reciveMsg(receivedShares, sizeToReceive*sizeof(__m128i));

	//save share of R^i (BGW share)
	RShare[party] = receivedShares[0];

	//add to local shares/shares from other parties
	mtxVal.lock();
		XORvectors(shares, receivedShares, shares, sizeToReceive);
	mtxVal.unlock();
	_aligned_free(receivedShares);
}

void addLocalSharesHME()
{

    __m128i* localShares= static_cast<__m128i *>(_aligned_malloc(totalSharesNumber*sizeof(__m128i), 16));


	computeSharesLatinCryptLocalHME(localShares);

	//save share of R^i (BGW share)
	RShare[partyNum] = localShares[0];

	//add to shares from other parties
	mtxVal.lock();
		XORvectors(shares, localShares, shares,totalSharesNumber);
	mtxVal.unlock();
	_aligned_free(localShares);
}

//

void generatePolynomialsLatinCryptHME()
{
	allSecrets= static_cast<__m128i *>(_aligned_malloc(totalSharesNumber*sizeof(__m128i), 16));
	int totalNumberOfCoef;

	{
		numOfSmallDegPolynomials = 1 + cyc->numOfANDGates + cyc->numOfInputWires;
        if (majType != 13 && majType != 14 && majType != 23 && majType != 24)
		    totalNumberOfCoef = numOfSmallDegPolynomials*degSmall+numOfParties*(totalSharesNumber-numOfSmallDegPolynomials);
        else
            totalNumberOfCoef = numOfSmallDegPolynomials*degSmall;
	}

	//choose random coefficients
	allCoefficients= static_cast<__m128i *>(_aligned_malloc(totalNumberOfCoef*sizeof(__m128i), 16));
	for (int i = 0; i < totalNumberOfCoef; i++)
		allCoefficients[i] = RAND;

	allSecrets[0] = R;

	int count = 1;//Start counting from 1 because 0 is shares of R.
	for (int p = 0; p < numOfParties; p++)
		for (int i = 0; i < cyc->playerArray[p].playerBitAmount; i++)
		{
			if (cyc->playerArray[p].playerWires[i]->lambda)
				allSecrets[count] = ONE;
			else
				allSecrets[count] = ZERO;
			count++;
		}

	for (int g = 0; g < cyc->numOfANDGates; g++)
	{
		if (cyc->regularGates[g]->output->lambda)
			allSecrets[count]= ONE;
		else
			allSecrets[count] = ZERO;
		count++;
	}



    if (majType != 13 && majType != 14 && majType != 23 && majType != 24)
    {
    	for (int g = 0; g < cyc->numOfANDGates; g++)
    		for (int i = 0; i < 4; i++)
    			for (int p = 0; p < numOfPartiesLC; p++)
    			{
    				allSecrets[count] = cyc->regularGates[g]->G[i][p];
    				count++;
    			}
    }
}

//
void computeSharesLatinCryptHME(int p, __m128i* ans)
{
	int count = 0;
	for (int i = 0; i < numOfSmallDegPolynomials; i++)
	{
		ans[i] = allSecrets[i];

		for (int deg = 1; deg < degSmall+1; deg++)
		{

			ans[i] = ADD(ans[i], MULHZ(allCoefficients[count], powers[p*degBig +(deg-1)]));
			count++;
		}
	}
	int offset=totalSharesNumber-numOfSmallDegPolynomials;

    count+=p*offset;
	for (int i = numOfSmallDegPolynomials; i < totalSharesNumber; i++)
	{
		ans[i]=allCoefficients[count];
		count++;
	}
}

void computeSharesLatinCryptLocalHME(__m128i* ans)
{
	int count = 0;
	for (int i = 0; i < numOfSmallDegPolynomials; i++)
	{
		ans[i] = allSecrets[i];

		for (int deg = 1; deg < degSmall+1; deg++)
		{

			ans[i] = ADD(ans[i], MULHZ(allCoefficients[count], powers[partyNum*degBig +(deg-1)]));
			count++;
		}
	}

    
    if (majType != 13 && majType != 14 && majType != 23 && majType != 24)
    {
    	int offset=totalSharesNumber-numOfSmallDegPolynomials;
    	for (int i = numOfSmallDegPolynomials; i < totalSharesNumber; i++)
    	{
    		ans[i]=allSecrets[i];//real secret

            if (partyNum<numOfPartiesLCNew)
    		{
        		for (int p = 0; p < numOfParties; p++)
        		{
        			if (p!=partyNum)
        				ans[i]=ADD(ans[i],allCoefficients[count+p*offset]);
        		}
            }
    		count++;
    	}
    }
}




inline void XORsuperseedsNewHME(__m128i *superseed1, __m128i *superseed2, __m128i *out)
{
	for (int p = 0; p < numOfPartiesLC; p++)
	{
		out[p] = _mm_xor_si128(superseed1[p], superseed2[p]);
	}
}

bool* computeOutputsHME()
{

    	__m128i* arr = static_cast<__m128i *>(_aligned_malloc(numOfPartiesLC * sizeof(__m128i), 16));
	bool* outputs = new bool[cyc->outputWires.playerBitAmount];
	bool valueIn1;
	bool valueIn2;
	for (int g = 0; g < cyc->amountOfGates; g++)
	{

		if (cyc->gateArray[g].flagNOMUL)
		{
			XORsuperseedsNewHME(cyc->gateArray[g].input1->superseed, cyc->gateArray[g].input2->superseed, cyc->gateArray[g].output->superseed);
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

			for (int p = 0; p < numOfPartiesLC; p++)
			{
				PSEUDO_RANDOM_FUNCTION(cyc->gateArray[g].input1->superseed[p], cyc->gateArray[g].input2->superseed[p], cyc->gateArray[g].gateNumber, numOfPartiesLC, arr);

				XORsuperseedsNewHME(cyc->gateArray[g].output->superseed, arr, cyc->gateArray[g].output->superseed);
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

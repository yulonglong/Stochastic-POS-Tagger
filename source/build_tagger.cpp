//build_tagger.cpp
//@author Steven Kester Yuwono
//@matric A0080415N


#include "Storage.cpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stack>
#include <map>
#include <vector>
#include <cmath>
#define DBL_MAX 1.7976931348623158e+308 /* max value */
#define TAGSIZE 47
#define SMALLTAGSIZE 45
using namespace std;

class build_tagger{
public:
	Storage storage;

	build_tagger(){
	}

	bool exportData(string filename){
		return storage.exportData(filename);
	}

	bool countData(string filename){
		ifstream infile;
		infile.open(filename.c_str(),ios::in);
		if(infile.fail()){
			return false;
		}

		string startTag = "<s>";
		string endTag = "</s>";

		string line;
		while(getline(infile,line)){
			//add tag count for first character <s>, mapped to index 45
			storage.tagCountTable[45]+=1;

			string prevTag = startTag;
			istringstream istream(line);
			string word, tag;
			string wordAndTag;
			//general case, counting the occurences of t(i),t(i-1)
			while (getline(istream,wordAndTag,' ')){
				storage.splitWordAndTag(wordAndTag,word,tag);
				//cout << word << " / " << tag << endl;
				int tagIndex = storage.getTagIndex(tag);
				int prevTagIndex = storage.getTagIndex(prevTag);
				storage.transitionTagCountTable[tagIndex][prevTagIndex]+=1;
				storage.tagCountTable[tagIndex] += 1;
				int wordIndex = storage.insertWord(word);
				storage.wordTagCountTable[wordIndex][tagIndex]+=1;
				storage.wordCountTable[wordIndex]+=1;
				prevTag = tag;
				storage.totalWordBag++;
			}
			//update for the sentence close tag
			int tagIndex = storage.getTagIndex(endTag);
			int prevTagIndex = storage.getTagIndex(prevTag);
			storage.transitionTagCountTable[tagIndex][prevTagIndex]+=1;
			//add tag count for last character </s>, mapped to index 46
			storage.tagCountTable[46]+=1;
		}

		storage.totalWordType = storage.words.size();

		infile.close();
		return true;
	}

	void addOneSmoothing(){
		//ADD ONE to all tables
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.transitionTagCountTable[i][j] += 1;
				storage.tagCountTable[i]+=1;
				storage.tagCountTable[j]+=1;
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.wordTagCountTable[i][j] += 1;
				storage.wordCountTable[i] += 1;
				storage.tagCountTable[j] += 1;
			}
		}
		//END ADD ONE
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				//probability table(i|j) = count(j,i) / count(j)
				//probability table(t(k)|t(k-1)) = count(t(k-1),t(k)) / count(t(k-1))
				storage.tagProbTable[i][j] = (double)storage.transitionTagCountTable[i][j] / (double)storage.tagCountTable[j];
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				//probability table(i|j) = count(j,i) / count(j)
				//probability table(w(k)|t(k)) = count(t(k),w(k)) / count(t(k))
				storage.wordTagProbTable[i][j] = (double)storage.wordTagCountTable[i][j] / (double)storage.tagCountTable[j];
			}
		}
		return;
	}

	void wittenBellSmoothing(){
		//Begin Initialize T and Z for t(k) and t(k-1) Bigram Probability
		int T_Tag[TAGSIZE];
		int Z_Tag[TAGSIZE];
		memset(T_Tag,0,sizeof(T_Tag));
		memset(Z_Tag,0,sizeof(Z_Tag));
		
		for(int j=0;j<TAGSIZE;j++){
			for(int k=0;k<TAGSIZE;k++){
				if (storage.transitionTagCountTable[k][j]>0){
					T_Tag[j]++;
				}
				else{
					Z_Tag[j]++;
				}
			}
		}
		//End Init

		//Calculate t(k) and t(k-1)  Bigram Probability
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				if (storage.transitionTagCountTable[i][j]>0){
					storage.tagProbTable[i][j] = (double)storage.transitionTagCountTable[i][j] / ((double)storage.tagCountTable[j] + (double)T_Tag[j]);
				}
				else{
					storage.tagProbTable[i][j] = (double)T_Tag[j] / ((double)Z_Tag[j] * ((double)storage.tagCountTable[j] + (double)T_Tag[j]));
				}
			}
		}
		//end calculate

		//Begin Initialize T and Z for w(k) and t(k) Probability
		int T_WordTag[TAGSIZE];
		int Z_WordTag[TAGSIZE];
		memset(T_WordTag,0,sizeof(T_WordTag));
		memset(Z_WordTag,0,sizeof(Z_WordTag));
		
		for(int j=0;j<TAGSIZE;j++){
			for(int k=0;k<storage.totalWordType;k++){
				if (storage.wordTagCountTable[k][j]>0){
					T_WordTag[j]++;
				}
				else{
					Z_WordTag[j]++;
				}
			}
		}
		//End Init

		//Calculate w(k) and t(k) Probability
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				if (storage.wordTagCountTable[i][j]>0){
					storage.wordTagProbTable[i][j] = (double)storage.wordTagCountTable[i][j] / ((double)storage.tagCountTable[j] + (double)T_WordTag[j]);
				}
				else{
					storage.wordTagProbTable[i][j] = (double)T_WordTag[j] / ((double)Z_WordTag[j] * ((double)storage.tagCountTable[j] + (double)T_WordTag[j]));
				}
			}
		}
		//end calculate
		return;
	}

	void kneserNeySmoothing(){
		double D = 0.75;

		double alphaTag[TAGSIZE];
		double alphaWordTag[TAGSIZE];

		double PContinuationTag[TAGSIZE];
		double PContinuationWordTag[storage.totalWordType];


		//BEGIN calculate continuation probability for tag bigram
		int tagNumerator[TAGSIZE];
		memset(tagNumerator,0,sizeof(tagNumerator));
	
		for(int i=0;i<TAGSIZE;i++){
			int continuationNumerator = 0;
			for(int j=0;j<TAGSIZE;j++){
				if(storage.transitionTagCountTable[i][j]>0){
					continuationNumerator+=1;
				}
			}
			//add-one smoothing for the numerator for <s>, zero words precedes that character.
			if(continuationNumerator == 0){
				tagNumerator[i] = 1;
			}
			else{
				tagNumerator[i] = continuationNumerator;
			}
		}
		int continuationDenominator = 0;
		for(int i=0;i<TAGSIZE;i++){
			continuationDenominator += tagNumerator[i];
		}
		//setting up Continuation probability
		for(int i=0;i<TAGSIZE;i++){
			PContinuationTag[i] = (double)tagNumerator[i] / (double)continuationDenominator;
		}
		//END calculate


		//BEGIN calculate continuation probability for Word and Tag
		int wordTagNumerator[storage.totalWordType];
		memset(wordTagNumerator,0,sizeof(wordTagNumerator));
	
		for(int i=0;i<storage.totalWordType;i++){
			int continuationNumerator = 0;
			for(int j=0;j<TAGSIZE;j++){
				if(storage.wordTagCountTable[i][j]>0){
					continuationNumerator+=1;
				}
			}
			//add-one smoothing for the numerator for <s>, zero words precedes that character.
			if(continuationNumerator == 0){
				wordTagNumerator[i] = 1;
			}
			else{
				wordTagNumerator[i] = continuationNumerator;
			}
		}

		continuationDenominator = 0;
		for(int i=0;i<storage.totalWordType;i++){
			continuationDenominator += wordTagNumerator[i];
		}

		//setting up Continuation probability
		for(int i=0;i<storage.totalWordType;i++){
			PContinuationWordTag[i] = (double)wordTagNumerator[i] / (double)continuationDenominator;
		}
		//END calculate


		//calculate alpha for Tags Bigram (normalization factor)
		for(int j=0;j<TAGSIZE;j++){
			double totalBigram = 0;
			for(int i=0;i<TAGSIZE;i++){
				if(storage.transitionTagCountTable[i][j]>0){
					storage.tagProbTable[i][j] = ((double)storage.transitionTagCountTable[i][j] - D) / (double)storage.tagCountTable[j];
					totalBigram += storage.tagProbTable[i][j];
				}
			}
			alphaTag[j] = (1.0 - totalBigram) / (1.0 - PContinuationTag[j]);
		}
		
		//calculate alpha for Word and Tag (normalization factor)
		for(int j=0;j<TAGSIZE;j++){
			double totalBigram = 0;
			for(int i=0;i<storage.totalWordType;i++){
				if(storage.wordTagCountTable[i][j]>0){
					storage.wordTagProbTable[i][j] = ((double)storage.wordTagCountTable[i][j] - D) / (double)storage.tagCountTable[j];
					totalBigram += storage.wordTagProbTable[i][j];
				}
			}
			alphaWordTag[j] = (1.0 - totalBigram) / (1.0 - PContinuationWordTag[j]);
		}




		//real probability calculation for Word give the preceding Words
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				if(storage.transitionTagCountTable[i][j]>0){
					storage.tagProbTable[i][j] = ((double)storage.transitionTagCountTable[i][j] - D) / (double)storage.tagCountTable[j];
				}
				else{
					storage.tagProbTable[i][j] = alphaTag[j] * PContinuationTag[i];
				}
			}
			//cout << PContinuationTag[i] << endl;
		}
		//real probability calculation for Word given Tag
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				if(storage.wordTagCountTable[i][j]>0){
					storage.wordTagProbTable[i][j] = ((double)storage.wordTagCountTable[i][j] - D)  / (double)storage.tagCountTable[j];
				}
				else{
					storage.wordTagProbTable[i][j] = alphaWordTag[j] * PContinuationWordTag[i];
				}
			}
		}

		return;

	}

	void processUnigramProbability(){
		for(int i=0;i<TAGSIZE;i++){
			storage.tagUnigramProbTable[i] = storage.tagCountTable[i] / (double) storage.totalWordBag;
		}
		for(int i=0;i<storage.totalWordType;i++){
			storage.wordTagUnigramProbTable.push_back(storage.wordCountTable[i] / (double) storage.totalWordBag);
		}
		return;
	}

	void processDataWithSmoothing(string smoothingType){
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<storage.totalWordType;i++){
			storage.wordTagProbTable.push_back(emptyVec);
		}
		smoothingType = storage.stringToLower(smoothingType);
		if(smoothingType == "addone"){
			cout << "Add-One Smoothing" << endl;
			addOneSmoothing();
		}
		else if(smoothingType == "wittenbell"){
			cout << "Witten-Bell Smoothing" << endl;
			wittenBellSmoothing();
		}
		else if(smoothingType == "kneserney"){
			cout << "Kneser-Ney Smoothing with backoff" << endl;
			kneserNeySmoothing();
		}
		//if invalid choice, use kneser-ney, the best smoothing
		else{
			cout << "Invalid smoothing type" << endl;
			cout << "using default smoothing : Kneser-Ney Smoothing with backoff" << endl;
			kneserNeySmoothing();
		}
		processUnigramProbability();
		return;
	}
};

class Validation{
public:
	Storage storage;

	Validation(Storage &newStorage){
		storage = newStorage;
	}

	//statistics table initialization
	double correctlyClassifiedInstances;

	int confusionMatrix[SMALLTAGSIZE][SMALLTAGSIZE];

	int totalInstances;

	//initialize table for True positive, False Positive, True Negative, False Negative, and total number of true instances for every class.
	int TP[SMALLTAGSIZE];
	int FP[SMALLTAGSIZE];
	int TN[SMALLTAGSIZE];
	int FN[SMALLTAGSIZE];
	int totalTrueInstances[SMALLTAGSIZE];
	
	//initialize TPRate(Recall), FP rate, Precision, F-Measure, and accuracy
	double TPrate[SMALLTAGSIZE];
	double FPrate[SMALLTAGSIZE];
	double precision[SMALLTAGSIZE];
	double Fmeasure[SMALLTAGSIZE];
	double accuracy[SMALLTAGSIZE];
	
	//calculate weighted average of TPRate(Recall), FP rate, Precision, F-Measure, and accuracy
	double weightedAveTPrate;
	double weightedAveFPrate;
	double weightedAvePrecision;
	double weightedAveFmeasure;
	double weightedAveAccuracy;

	void refresh(){
		correctlyClassifiedInstances = 0;
		totalInstances = 0;
		memset(confusionMatrix,0,sizeof(confusionMatrix));
		memset(TP,0,sizeof(TP));
		memset(FP,0,sizeof(FP));
		memset(TN,0,sizeof(TN));
		memset(FN,0,sizeof(FN));
		memset(totalTrueInstances,0,sizeof(totalTrueInstances));
		memset(TPrate,0,sizeof(TPrate));
		memset(FPrate,0,sizeof(FPrate));
		memset(precision,0,sizeof(precision));
		memset(Fmeasure,0,sizeof(Fmeasure));
		memset(accuracy,0,sizeof(accuracy));
		weightedAveTPrate = 0;
		weightedAveFPrate = 0;
		weightedAvePrecision = 0;
		weightedAveFmeasure = 0;
		weightedAveAccuracy = 0;
	}

	//end statistics table initialization

	//methods for statistics 
	bool processConfusionMatrix(FILE* outfile, bool showConfusionMatrix){
		memset(confusionMatrix,0,sizeof(confusionMatrix));
		for(int z=0;z<(int)correctTagOutput.size();z++){
			vector<string> lineCorrectTagOutput = correctTagOutput[z];
			vector<string> lineTagOutput = tagOutput[z];
			for(int i=0;i<(int)lineCorrectTagOutput.size();i++){
				//set confusion matrix
				int tagIndex = storage.getTagIndex(lineTagOutput[i]);
				int correctTagIndex = storage.getTagIndex(lineCorrectTagOutput[i]);
				confusionMatrix[correctTagIndex][tagIndex] +=1;
			}
		}

		if(showConfusionMatrix){
			//print confusion matrix
			fprintf(outfile,"\n=== Confusion Matrix ===\n\n");
			for(int i=0;i<SMALLTAGSIZE;i++){
				fprintf(outfile, "%4d ",i);
			}
			fprintf(outfile, "\n");
			for(int i=0;i<SMALLTAGSIZE;i++){
				for(int j=0;j<SMALLTAGSIZE;j++){
					fprintf(outfile,"%4d ",confusionMatrix[i][j]);
				}
				fprintf(outfile,"| %d = %s\n",i,storage.indexTags[i].c_str());
			}
			fprintf(outfile,"\n");
		}
		return true;
	}

	bool processStatistics(FILE* outfile, bool showStatistics){
		
		//count total number of True (class) instances, together with TP, and FN
		for(int i=0;i<SMALLTAGSIZE;i++){
			for(int j=0;j<SMALLTAGSIZE;j++){
				if(i==j){
					TP[i] = confusionMatrix[i][j];
				}
				else{
					FN[i] += confusionMatrix[i][j];
				}
				totalTrueInstances[i] += confusionMatrix[i][j];
			}
		}
		//count FP
		for(int j=0;j<SMALLTAGSIZE;j++){
			for(int i=0;i<SMALLTAGSIZE;i++){
				if(i!=j){
					FP[j] += confusionMatrix[i][j];
				}
			}
		}
		//count Total number of instances
		for(int i=0;i<SMALLTAGSIZE;i++){
			//cout << storage.indexTags[i] << " " << totalTrueInstances[i] << endl;
			totalInstances += totalTrueInstances[i];
		}
		//count TN
		for(int i=0;i<SMALLTAGSIZE;i++){
			TN[i] = totalInstances - TP[i] - FP[i] - FN[i];
		}


		//calculate TPRate(Recall), FP rate, Precision, and F-Measure, and accuracy
		for(int i=0;i<SMALLTAGSIZE;i++){
			if(totalTrueInstances[i] > 0){
				if(TP[i] + FN[i] > 0){
					TPrate[i] = (double)TP[i] / ((double)TP[i] + (double)FN[i]);
				}
				if(FP[i] + TN[i] > 0){
					FPrate[i] = (double)FP[i] / ((double)FP[i] + (double)TN[i]);
				}
				if(TP[i] + FP[i] > 0){
					precision[i] = (double)TP[i] / ((double)TP[i] + (double)FP[i]);
				}
				if(TPrate[i] + precision[i] > 0){
					Fmeasure[i] = (2.0 * TPrate[i] * precision[i]) / (TPrate[i] + precision[i]);
				}
				accuracy[i] = ((double)TP[i] + (double)TN[i]) / (double)totalInstances;
			}
		}

		for(int i=0;i<SMALLTAGSIZE;i++){
			weightedAveTPrate += (double)totalTrueInstances[i] * TPrate[i];
			weightedAveFPrate += (double)totalTrueInstances[i] * FPrate[i];
			weightedAvePrecision += (double)totalTrueInstances[i] * precision[i];
			weightedAveFmeasure += (double)totalTrueInstances[i] * Fmeasure[i];
			weightedAveAccuracy += (double)totalTrueInstances[i] * accuracy[i];
		}
		weightedAveTPrate /= (double)totalInstances;
		weightedAveFPrate /= (double)totalInstances;
		weightedAvePrecision /= (double)totalInstances;
		weightedAveFmeasure /= (double)totalInstances;
		weightedAveAccuracy /= (double)totalInstances;

		if(showStatistics){
			fprintf(outfile,"\n === Detailed Accuracy By Class === \n\n");
			fprintf(outfile,"               TP Rate   FP Rate   Precision   F-Measure   Accuracy   Total Instance   Class\n");
			for(int i=0;i<SMALLTAGSIZE;i++){
				fprintf(outfile,"               %.5f   %.5f    %.5f     %.5f    %.6f       %8d     %s\n", TPrate[i],FPrate[i],precision[i],Fmeasure[i],accuracy[i],totalTrueInstances[i],storage.indexTags[i].c_str());
			}
			fprintf(outfile,"Weighted Avg.  %.5f   %.5f    %.5f     %.5f    %.6f       %8d      \n", weightedAveTPrate,weightedAveFPrate,weightedAvePrecision,weightedAveFmeasure,weightedAveAccuracy,totalInstances);
			fprintf(outfile,"\n\n");
		}
		return true;
	}

	//process the wrong word and output it to outfile if showWrongWord is true.
	bool processWrongWord(FILE* outfile,bool showWrongWord){

		if(showWrongWord){
			fprintf(outfile,"\n === Wrong instances === \n\n");
		}

		int correctInstance = 0;
		int wrongInstance = 0;

		for(int z=0;z<(int)correctTagOutput.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> lineCorrectTagOutput = correctTagOutput[z];
			vector<string> lineTagOutput = tagOutput[z];
			for(int i=0;i<(int)lineCorrectTagOutput.size();i++){
				//count number of correct instances
				if(lineCorrectTagOutput[i]==lineTagOutput[i]){
					correctInstance += 1;
				}
				else{
					wrongInstance += 1;
					//print the wrong instances
					if(showWrongWord){
						fprintf(outfile,"%s : %s (correct: %s)\n",lineInput[i].c_str(),lineTagOutput[i].c_str(),lineCorrectTagOutput[i].c_str());
					}
				}
			}
		}

		correctlyClassifiedInstances = ((double)correctInstance/((double)correctInstance + (double)wrongInstance))*100.0;
		if(showWrongWord){
			fprintf(outfile,"\n\n");
			fprintf(outfile, "Correct Instances: %d\n",correctInstance);
			fprintf(outfile, "Wrong Instances: %d\n",wrongInstance);
			fprintf(outfile, "Correctly Classified Instances: %.2f %%\n",correctlyClassifiedInstances);
		}
		return true;
	}


	bool trainingStatistics(string filename, bool showWrongWord, bool showConfusionMatrix, bool showStatistics){
		FILE* outfile;
		outfile = fopen(filename.c_str(),"w+");
		if(outfile == NULL){
			return false;
		}

		//init
		refresh();
		//process
		processWrongWord(outfile,showWrongWord);
		processConfusionMatrix(outfile,showConfusionMatrix);
		processStatistics(outfile,showStatistics);
		
		fclose(outfile);
		return true;
	}
	//end methods for statistics


	//initialize the real identifier algorithm (viterbi)
	vector< vector<string> > inputSentences;
	vector< vector<string> > correctTagOutput;
	vector< vector<string> > tagOutput;

	bool readInputWithTag(string filename){
		ifstream infile;
		infile.open(filename.c_str(),ios::in);
		if(infile.fail()){
			return false;
		}
		string line;
		while(getline(infile,line)){
			vector<string> lineInput;
			vector<string> lineTagOutput;
			istringstream istream(line);
			string word, tag;
			string wordAndTag;
			while (getline(istream,wordAndTag,' ')) {
				storage.splitWordAndTag(wordAndTag,word,tag);
				lineInput.push_back(word);
				lineTagOutput.push_back(tag);
			}
			inputSentences.push_back(lineInput);
			correctTagOutput.push_back(lineTagOutput);
		}
		infile.close();
		return true;
	}

	void initViterbi(vector< vector<double> > &dp, int inputSize, vector< vector<int> > &parent) {
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<inputSize;i++){
			dp.push_back(emptyVec);
		}

		parent.clear();
		vector<int> emptyVecInt (SMALLTAGSIZE,0);
		for(int i=0;i<=inputSize+1;i++){
			parent.push_back(emptyVecInt);
		}
		return;
	}

	vector<string> viterbiAlgorithm(vector<string> &input){
		int inputSize = input.size();
		//initializing dp table and parent/backpointer table
		vector< vector<double> > dp;
		vector< vector<int> > parent;

		initViterbi(dp,inputSize,parent);

		//first loop to initialize the first state
		//i.e. connecting the start node to the first state nodes.
		for(int i=0;i<SMALLTAGSIZE;i++){
			parent[0][i] = 45;
		}
		int wordIndex = storage.getWordIndex(input[0]);
		for(int i=0;i<SMALLTAGSIZE;i++){
			dp[0][i]=log2(tagNewProbTable[i][45])+log2(wordTagNewProbTable[wordIndex][i]);
		}

		//Dynamic programming to set rest of the states
		for(int d=1;d<inputSize;d++){
			wordIndex = storage.getWordIndex(input[d]);
			for(int i=0;i<SMALLTAGSIZE;i++){
				double maxPrev = -DBL_MAX;
				//get the maximum from the previous nodes
				for(int j=0;j<SMALLTAGSIZE;j++){
					double tempDouble = dp[d-1][j]+log2(tagNewProbTable[i][j]);
					//cout << tempDouble << endl;
					if(maxPrev<tempDouble){
						maxPrev = tempDouble;
						parent[d][i]=j;
					}
				}
				//store the result of the maximum/optimum probability in the state node
				dp[d][i]=maxPrev+log2(wordTagNewProbTable[wordIndex][i]);
			}
		}

		//last loop to get the maximum/optimum value of the last state
		//i.e. connecting the last state nodes to the end node
		double maxPrev = -DBL_MAX;
		for(int j=0;j<SMALLTAGSIZE;j++){
			double tempDouble = dp[input.size()-1][j]+log2(tagNewProbTable[46][j]);
			if(maxPrev<tempDouble){
				maxPrev = tempDouble;
				for(int i=0;i<SMALLTAGSIZE;i++){
					parent[inputSize][i]=j;
				}
			}
		}

		vector<string> tagResult;

		//obtain tag indexes from viterbi backpointer (i named it parent) and store in tagOutput
		stack<int> s;
		int prevBestTag = parent[inputSize][0];
		s.push(prevBestTag);
		for(int d=inputSize-1;d>0;d--){
			s.push(parent[d][prevBestTag]);
			prevBestTag = parent[d][prevBestTag];
		}
		while(!s.empty()){
			tagResult.push_back(storage.indexTags[s.top()]);
			s.pop();
		}
		return tagResult;
	}

	//process the training data and get statistics from the development data, without interpolation
	void processDevelopmentDataWithoutInterpolation(){
		refreshInterpolationScore();
		
		setInterpolationTable(100);
		tagOutput.clear();
		for(int z=0;z<(int)inputSentences.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> tagResult = viterbiAlgorithm(lineInput);
			tagOutput.push_back(tagResult);
		}
	}

	//process the training data and get statistics from the development data, with interpolation and find the best interpolation weight
	//the parameter is the lower bound index, minimum 0, and upper bound index, maximum is 100.
	void processDevelopmentDataWithInterpolation(int lowerBoundIndex, int upperBoundIndex){
		refreshInterpolationScore();
		
		//loop through hundred possible interpolation weights and get recall value for each weight
		for(int i = lowerBoundIndex ; i <= upperBoundIndex ; i++){
			cout << "interpolating weight index : " << i << endl;
			setInterpolationTable(i);
			tagOutput.clear();
			for(int z=0;z<(int)inputSentences.size();z++){
				vector<string> lineInput = inputSentences[z];
				vector<string> tagResult = viterbiAlgorithm(lineInput);
				tagOutput.push_back(tagResult);
			}

			processWrongWord(NULL,false);
			interpolationRecall[i] = correctlyClassifiedInstances;
		}

		double maxRecall = -1;
		int maxRecallIndex = -1;
		for(int i = lowerBoundIndex ; i <= upperBoundIndex ; i++){
			if(maxRecall<interpolationRecall[i]){
				maxRecall = interpolationRecall[i];
				maxRecallIndex = i;
			}
		}
		optimumInterpolationIndex = maxRecallIndex;

		//run viterbi one more time to keep the best result in the statistics
		cout << "best interpolation weight index: " << optimumInterpolationIndex << endl;
		printf("lambda 1 : %.2f (used with bigram probability, i.e. p(x|y)\n",(double)optimumInterpolationIndex/100.0);
		printf("lambda 2 : %.2f (used with unigram probability, p(x)\n",(double)(100-optimumInterpolationIndex)/100.0);

		setInterpolationTable(optimumInterpolationIndex);
		tagOutput.clear();
		for(int z=0;z<(int)inputSentences.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> tagResult = viterbiAlgorithm(lineInput);
			tagOutput.push_back(tagResult);
		}

	}

	void copyInterpolationIndexToStorage(Storage &newStorage){
		setInterpolationTable(optimumInterpolationIndex);
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				newStorage.tagProbTable[i][j] = tagNewProbTable[i][j];
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				newStorage.wordTagProbTable[i][j] = wordTagNewProbTable[i][j];
			}
		}
		return;
	}
	//end identifier algorithm


	//start interpolation
	double interpolationWeight[101];
	double interpolationRecall[101];
	int optimumInterpolationIndex;

	double tagNewProbTable[TAGSIZE][TAGSIZE];
	vector< vector<double> > wordTagNewProbTable;

	void refreshInterpolationScore(){
		optimumInterpolationIndex = 100;
		for(int i=0;i<101;i++){
			interpolationWeight[i] = (double)i / 100.0;
		}
		memset(interpolationRecall,0,sizeof(interpolationRecall));
		return;
	}

	void refreshInterpolationProbTable(){
		memset(tagNewProbTable,0,sizeof(tagNewProbTable));
		wordTagNewProbTable.clear();
		for(int i=0;i<storage.totalWordType;i++){
			vector<double> emptyVec (TAGSIZE,0);
			wordTagNewProbTable.push_back(emptyVec);
		}
		return;
	}

	void setInterpolationTable(int index){
		refreshInterpolationProbTable();
		double interpolationUnigramWeight = 1.0 - interpolationWeight[index];
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				tagNewProbTable[i][j] = (storage.tagProbTable[i][j] * interpolationWeight[index]) + (interpolationUnigramWeight * storage.tagUnigramProbTable[i]);
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				wordTagNewProbTable[i][j] = (storage.wordTagProbTable[i][j] * interpolationWeight[index]) + (interpolationUnigramWeight * storage.wordTagUnigramProbTable[i]);
			}
		}
		return;
	}


	//end interpolation

};


int main(int argc, char* argv[]){
	string trainingFilename = argv[1];
	string devtFilename = argv[2];
	string modelFilename = argv[3];
	
	//create build_tagger instance
	build_tagger bt;

	cout << endl;
	cout << "Reading Training file : " << trainingFilename << endl;
	cout << "Counting attributes from the training file..." << endl;
	bt.countData(trainingFilename);

	cout << endl;
	cout << "Processing data, calculating probabilities with smoothing : " << endl;
	//there are three type of smoothing "addone", "wittenbell", or "kneserney";
	bt.processDataWithSmoothing("kneserney");
	cout << endl;

	//create Validation instance
	Validation v(bt.storage);

	cout << "Reading Development/Tuning file : " << devtFilename << endl;
	v.readInputWithTag(devtFilename);
	cout << endl;

	//change this to false not to use interpolation (faster processing speed without interpolation)
	bool useInterpolation = false;

	if(useInterpolation){
		cout << "Training and enhancing POS Tagger with interpolation." <<  endl;
		//parameter is needed to specify the range of interpolation weights, max range is (0,100)
		v.processDevelopmentDataWithInterpolation(40,90);
	}
	else{
		cout << "Training and enhancing POSTagger without interpolation." <<  endl;
		v.processDevelopmentDataWithoutInterpolation();
	}

	cout << endl;
	cout << "Updating probability table for enhancement using interpolation result." << endl;
	v.copyInterpolationIndexToStorage(bt.storage);

	//@args:
	//	first param (string): Statistics filename to be created,
	//  second param (bool) : set to true to show wrong words identified by the POS Tagger program
	//  third param  (bool) : set to true to show confusion matrix
	//  fourth param (bool) : set to true to show detailed statistics such as Recall, TP rate, FP rate, F-measure, precision, accuracy and their weighted average.
	//uncomment the line below to output/print the statistics
	v.trainingStatistics("outstat.txt",true,true,true);

	cout << endl;
	cout << "Exporting data to file : " << modelFilename << endl;
	bt.exportData(modelFilename);

	cout << endl;
	cout << "Done! build_tagger is terminated." << endl;
	cout << endl;

	return 0;
}
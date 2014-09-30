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

		//calculate alpha for Tags Bigram (normalization factor)
		for(int j=0;j<TAGSIZE;j++){
			double totalBigram = 0;
			double totalUnigram = 0;
			for(int i=0;i<TAGSIZE;i++){
				if(storage.transitionTagCountTable[i][j]>0){
					storage.tagProbTable[i][j] = ((double)storage.transitionTagCountTable[i][j] - D) / (double)storage.tagCountTable[j];
					totalBigram += storage.tagProbTable[i][j];
				}
				else{
					totalUnigram += storage.tagCountTable[i] / (double) storage.totalWordBag;
				}
			}
			alphaTag[j] = (1.0 - totalBigram) / totalUnigram;
		}
		
		//calculate alpha for Word and Tag (normalization factor)
		for(int j=0;j<TAGSIZE;j++){
			double totalBigram = 0;
			double totalUnigram = 0;
			for(int i=0;i<storage.totalWordType;i++){
				if(storage.wordTagCountTable[i][j]>0){
					storage.wordTagProbTable[i][j] = ((double)storage.wordTagCountTable[i][j] - D) / (double)storage.tagCountTable[j];
					totalBigram += storage.wordTagProbTable[i][j];
				}
				else{
					totalUnigram += storage.wordCountTable[i] / (double) storage.totalWordBag;
				}
			}
			alphaWordTag[j] = (1.0 - totalBigram) / totalUnigram;
		}


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


		//EBGIN calculate continuation probability for Word and Tag
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

	void processData(){
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<storage.totalWordType;i++){
			storage.wordTagProbTable.push_back(emptyVec);
		}
		//addOneSmoothing();
		//wittenBellSmoothing();
		kneserNeySmoothing();
		return;
	}
};

class Validation{
public:
	Storage storage;

	vector< vector<string> > inputSentences;
	vector< vector<string> > correctTagOutput;
	vector< vector<string> > tagOutput;

	Validation(Storage &newStorage){
		storage = newStorage;
	}

	bool readInputWithTag(string filename){
		ifstream infile;
		infile.open(filename.c_str(),ios::in);
		if(infile.fail()){
			return false;
		}
		string line;
		int i = 0;
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

	void initViterbi(vector< vector<double> > &dp, int parent[], int inputSize){
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<inputSize;i++){
			dp.push_back(emptyVec);
		}
		memset(parent,0,sizeof(parent));
		return;
	}

	vector<string> viterbiAlgorithm(vector<string> &input){
		int inputSize = input.size();
		//initializing dp table and parent/backpointer table
		vector< vector<double> > dp;
		int parent[inputSize+1];

		initViterbi(dp,parent,inputSize);

		//first loop to initialize the first state
		//i.e. connecting the start node to the first state nodes.
		parent[0] = 45;
		int wordIndex = storage.getWordIndex(input[0]);
		for(int i=0;i<SMALLTAGSIZE;i++){
			dp[0][i]=log2(storage.tagProbTable[i][45])+log2(storage.wordTagProbTable[wordIndex][i]);
		}

		//Dynamic programming to set rest of the states
		for(int d=1;d<inputSize;d++){
			wordIndex = storage.getWordIndex(input[d]);
			for(int i=0;i<SMALLTAGSIZE;i++){
				double maxPrev = -DBL_MAX;
				//get the maximum from the previous nodes
				for(int j=0;j<SMALLTAGSIZE;j++){
					double tempDouble = dp[d-1][j]+log2(storage.tagProbTable[i][j]);
					//cout << tempDouble << endl;
					if(maxPrev<tempDouble){
						maxPrev = tempDouble;
						parent[d]=j;
					}
				}
				//store the result of the maximum/optimum probability in the state node
				dp[d][i]=maxPrev+log2(storage.wordTagProbTable[wordIndex][i]);
			}
		}

		//last loop to get the maximum/optimum value of the last state
		//i.e. connecting the last state nodes to the end node
		double maxPrev = -DBL_MAX;
		for(int j=0;j<SMALLTAGSIZE;j++){
			double tempDouble = dp[input.size()-1][j]+log2(storage.tagProbTable[46][j]);
			if(maxPrev<tempDouble){
				maxPrev = tempDouble;
				parent[input.size()]=j;
			}
		}

		vector<string> tagResult;

		//obtain tag indexes from viterbi backpointer (i named it parent) and store in tagOutput
		stack<int> s;
		for(int d=inputSize;d>0;d--){
			s.push(parent[d]);
		}
		while(!s.empty()){
			tagResult.push_back(storage.indexTags[s.top()]);
			s.pop();
		}
		return tagResult;
	}

	void processData(){
		for(int z=0;z<(int)inputSentences.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> tagResult = viterbiAlgorithm(lineInput);
			tagOutput.push_back(tagResult);
		}
	}

	bool printOutput(string filename){
		FILE* outfile;
		outfile = fopen(filename.c_str(),"w+");
		if(outfile == NULL){
			return false;
		}
		for(int z=0;z<(int)inputSentences.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> lineTagOutput = tagOutput[z];
			for(int i=0;i<(int)lineInput.size();i++){
				fprintf(outfile,"%s/%s ",lineInput[i].c_str(),lineTagOutput[i].c_str());
			}
			fprintf(outfile,"\n");
		}
		fclose(outfile);
		return true;
	}

	bool validateOutput(string filename){
		FILE* outfile;
		outfile = fopen(filename.c_str(),"w+");
		if(outfile == NULL){
			return false;
		}

		//init confusion Matrix
		int confusionMatrix[SMALLTAGSIZE][SMALLTAGSIZE];
		memset(confusionMatrix,0,sizeof(confusionMatrix));

		int correctInstance = 0;
		int wrongInstance = 0;

		for(int z=0;z<(int)correctTagOutput.size();z++){
			vector<string> lineInput = inputSentences[z];
			vector<string> lineCorrectTagOutput = correctTagOutput[z];
			vector<string> lineTagOutput = tagOutput[z];
			for(int i=0;i<(int)lineCorrectTagOutput.size();i++){
				//set confusion matrix
				int tagIndex = storage.getTagIndex(lineTagOutput[i]);
				int correctTagIndex = storage.getTagIndex(lineCorrectTagOutput[i]);
				confusionMatrix[correctTagIndex][tagIndex] +=1;

				//count number of correct instances
				if(lineCorrectTagOutput[i]==lineTagOutput[i]){
					correctInstance += 1;
				}
				else{
					wrongInstance += 1;
					//print the wrong instances
					fprintf(outfile,"%s : %s (correct: %s)\n",lineInput[i].c_str(),lineTagOutput[i].c_str(),lineCorrectTagOutput[i].c_str());
				}
			}
			fprintf(outfile,"\n");
		}

		fprintf(outfile, "Correct Instances: %d\n",correctInstance);
		fprintf(outfile, "Wrong Instances: %d\n",wrongInstance);
		double result = ((double)correctInstance/((double)correctInstance + (double)wrongInstance))*100.0;
		fprintf(outfile, "Correctly Classified Instances: %.2f %%\n",result);



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

		//initialize table for True positive, False Positive, True Negative, False Negative, and total number of true instances for every class.
		int TP[SMALLTAGSIZE];
		memset(TP,0,sizeof(TP));
		int FP[SMALLTAGSIZE];
		memset(FP,0,sizeof(FP));
		int TN[SMALLTAGSIZE];
		memset(TN,0,sizeof(TN));
		int FN[SMALLTAGSIZE];
		memset(FN,0,sizeof(FN));
		int totalTrueInstances[SMALLTAGSIZE];
		memset(totalTrueInstances,0,sizeof(totalTrueInstances));
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
		int totalInstances = 0;
		for(int i=0;i<SMALLTAGSIZE;i++){
			//cout << storage.indexTags[i] << " " << totalTrueInstances[i] << endl;
			totalInstances += totalTrueInstances[i];
		}
		//count TN
		for(int i=0;i<SMALLTAGSIZE;i++){
			TN[i] = totalInstances - TP[i] - FP[i] - FN[i];
			// cout << "TP " << TP[i] <<endl;
			// cout << "FP " << FP[i] <<endl;
			// cout << "TN " << TN[i] <<endl;
			// cout << "FN " << FN[i] <<endl;
		}

		//initialize TPRate(Recall), FP rate, Precision, and F-Measure
		double TPrate[SMALLTAGSIZE];
		memset(TPrate,0,sizeof(TPrate));
		double FPrate[SMALLTAGSIZE];
		memset(FPrate,0,sizeof(FPrate));
		double precision[SMALLTAGSIZE];
		memset(precision,0,sizeof(precision));
		double Fmeasure[SMALLTAGSIZE];
		memset(Fmeasure,0,sizeof(Fmeasure));

		//calculate TPRate(Recall), FP rate, Precision, and F-Measure
		for(int i=0;i<SMALLTAGSIZE;i++){
			if(totalTrueInstances[i]>0){
				TPrate[i] = (double)TP[i] / ((double)TP[i] + (double)FN[i]);
				FPrate[i] = (double)FP[i] / ((double)FP[i] + (double)TN[i]);
				precision[i] = (double)TP[i] / ((double)TP[i] + (double)FP[i]);
				Fmeasure[i] = (2.0 * TPrate[i] * precision[i]) / (TPrate[i] + precision[i]);
				// cout << "TPrate " << TPrate[i] << endl;
				// cout << "FPrate " << FPrate[i] << endl;
				// cout << "precision " << precision[i] << endl;
				// cout << "Fmeasure " << Fmeasure[i] << endl;
			}
		}

		//calculate weighted average of TPRate(Recall), FP rate, Precision, and F-Measure
		double weightedAveTPrate = 0;
		double weightedAveFPrate = 0;
		double weightedAvePrecision = 0;
		double weightedAveFmeasure = 0;
		for(int i=0;i<SMALLTAGSIZE;i++){
			weightedAveTPrate += (double)totalTrueInstances[i] * TPrate[i];
			weightedAveFPrate += (double)totalTrueInstances[i] * FPrate[i];
			weightedAvePrecision += (double)totalTrueInstances[i] * precision[i];
			weightedAveFmeasure += (double)totalTrueInstances[i] * Fmeasure[i];
		}
		weightedAveTPrate /= (double)totalInstances;
		weightedAveFPrate /= (double)totalInstances;
		weightedAvePrecision /= (double)totalInstances;
		weightedAveFmeasure /= (double)totalInstances;

		// cout << "TPrate " << weightedAveTPrate << endl;
		// cout << "FPrate " << weightedAveFPrate << endl;
		// cout << "precision " << weightedAvePrecision << endl;
		// cout << "Fmeasure " << weightedAveFmeasure << endl;

		fprintf(outfile,"\n === Detailed Accuracy By Class === \n\n");
		fprintf(outfile,"               TP Rate   FP Rate   Precision   F-Measure   Total Instance   Class\n");
		for(int i=0;i<SMALLTAGSIZE;i++){
			fprintf(outfile,"               %.5f   %.5f    %.5f     %.5f       %8d      %s\n", TPrate[i],FPrate[i],precision[i],Fmeasure[i],totalTrueInstances[i],storage.indexTags[i].c_str());
		}
		fprintf(outfile,"Weighted Avg.  %.5f   %.5f    %.5f     %.5f       %8d      \n", weightedAveTPrate,weightedAveFPrate,weightedAvePrecision,weightedAveFmeasure,totalInstances);
		

		fclose(outfile);
		return true;
	}

};


int main(int argc, char* argv[]){
	string trainingFilename = argv[1];
	string devtFilename = argv[2];
	string modelFilename = argv[3];
	
	build_tagger bt;

	bt.countData(trainingFilename);
	bt.processData();
	bt.exportData(modelFilename);

	Validation v(bt.storage);
	v.readInputWithTag(devtFilename);
	v.processData();
	v.validateOutput("outdev.txt");
	
	return 0;
}
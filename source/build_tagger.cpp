//build_tagger.cpp
//@author Steven Kester Yuwono
//@matric A0080415N

#include "Storage.cpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#define TAGSIZE 47
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

		//add one more word "---unknown---" to handle unknown words.
		int wordIndex = storage.insertWord("---unknown---");
		storage.wordCountTable[wordIndex]+=1;

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

		//calculate alpha for tag bigram
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
		
		//calculate alpha for word tag
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


		//calculate continuation probability for tag
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


		//calculate continuation probability for wordtag
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

		//real probability counting
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


int main(int argc, char* argv[]){
	string trainingFilename = argv[1];
	string devtFilename = argv[2];
	string modelFilename = argv[3];
	
	build_tagger bt;

	bt.countData(trainingFilename);
	bt.processData();
	bt.exportData(modelFilename);
	
	return 0;
}
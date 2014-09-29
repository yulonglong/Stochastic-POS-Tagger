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

	void countData(ifstream &infile){
		string line;
		while(getline(infile,line)){
			string prevTag = "<s>";
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
			tag = "</s>";
			int tagIndex = storage.getTagIndex(tag);
			int prevTagIndex = storage.getTagIndex(prevTag);
			storage.transitionTagCountTable[tagIndex][prevTagIndex]+=1;
		}
		storage.totalWordType = storage.words.size();
		return;
	}

	void exportData(ofstream &outfile){
		outfile << "Total Word Bag :" << storage.totalWordBag << endl;
		outfile << "Total Word Type :" << storage.totalWordType << endl;
		outfile << "Matrix of t(i-1) against t(i):" << endl;
		for(int i=0;i<TAGSIZE;i++){
			outfile << storage.indexTags[i] << " ";
			for(int j=0;j<TAGSIZE;j++){
				outfile << storage.tagProbTable[i][j] << " ";
			}
			outfile << endl;
		}
		outfile << "Matrix of t(i) against w(i):" << endl;
		for(int i=0;i<storage.totalWordType;i++){
			outfile << storage.indexWords[i] << " ";
			for(int j=0;j<TAGSIZE;j++){
				outfile << storage.wordTagProbTable[i][j] << " ";
			}
			outfile << endl;
		}
		return;
	}

	void addOneSmoothing(){
		//ADD ONE SMOOTHING
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
		//END ADD ONE SMOOTHING
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.tagProbTable[i][j] = (double)storage.transitionTagCountTable[i][j]/(double)storage.tagCountTable[j];
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.wordTagProbTable[i][j] = (double)storage.wordTagCountTable[i][j]/(double)storage.tagCountTable[j];
			}
		}
		return;
	}

	void processData(){
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<storage.totalWordType;i++){
			storage.wordTagProbTable.push_back(emptyVec);
		}
		addOneSmoothing();
		return;
	}
};


int main(int argc, char* argv[]){
	string trainingFilename = argv[1];
	string devtFilename = argv[2];
	string modelFilename = argv[3];
	ifstream infile;
	ofstream outfile;

	infile.open(trainingFilename.c_str(),ios::in);
	if(infile.fail()){
		return 0;
	}

	outfile.open(modelFilename.c_str(),ios::out);
	if(outfile.fail()){
		return 0;
	}

	build_tagger bt;

	bt.countData(infile);
	bt.processData();
	bt.exportData(outfile);
	

	return 0;
}
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
using namespace std;


class run_tagger{
public:
	Storage storage;

	void importData(ifstream &infile){
		string temp;
		string tempString;

		getline(infile,temp,':');//ignore string before colon
		getline(infile,tempString);
		storage.totalWordBag = atoi(tempString.c_str());
		getline(infile,temp,':');//ignore string before colon
		getline(infile,tempString);
		storage.totalWordType = atoi(tempString.c_str());
		getline(infile,temp);//ignore one line

		for(int i=0;i<TAGSIZE;i++){
			infile >> temp;
			for(int j=0;j<TAGSIZE;j++){
				infile >> storage.tagProbTable[i][j];
			}
		}
		getline(infile,temp);//ignore from cin
		getline(infile,temp);//ignore one line
		for(int i=0;i<storage.totalWordType;i++){
			string word;
			infile >> word;
			storage.insertWord(word);
			vector<double> emptyVec (TAGSIZE,0);
			storage.wordTagProbTable.push_back(emptyVec);
			for(int j=0;j<TAGSIZE;j++){
				infile >> storage.wordTagProbTable[i][j];
			}
		}
		return;
	}


	vector<string> input;
	vector<string> tagOutput;

	void readInput(ifstream &infile){
		string word;
		while(getline(infile,word,' ')){
			input.push_back(word);
		}
		return;
	}

	void viterbiAlgorithm(){
		//initializing dp table
		vector< vector<double> > dp;
		vector<double> emptyVec (TAGSIZE,0);
		for(int i=0;i<(int)input.size();i++){
			dp.push_back(emptyVec);
		}
		int parent[input.size()+1];

		//first loop
		parent[0] = 45;
		int wordIndex = storage.getWordIndex(input[0]);
		for(int i=0;i<TAGSIZE-2;i++){
			dp[0][i]=log2(storage.tagProbTable[i][45])+log2(storage.wordTagProbTable[wordIndex][i]);
		}

		//real DP
		for(int d=1;d<(int)input.size();d++){
			wordIndex = storage.getWordIndex(input[d]);
			for(int i=0;i<TAGSIZE-2;i++){
				double maxPrev = -DBL_MAX;
				//get the maximum from the previous nodes
				for(int j=0;j<TAGSIZE-2;j++){
					double tempDouble = dp[d-1][j]+log2(storage.tagProbTable[i][j]);
					//cout << tempDouble << endl;
					if(maxPrev<tempDouble){
						maxPrev = tempDouble;
						parent[d]=j;
					}
				}
				dp[d][i]=maxPrev+log2(storage.wordTagProbTable[wordIndex][i]);
			}
		}

		double maxPrev = -DBL_MAX;
		for(int j=0;j<TAGSIZE-2;j++){
			double tempDouble = dp[input.size()-1][j]+log2(storage.tagProbTable[46][j]);
			if(maxPrev<tempDouble){
				maxPrev = tempDouble;
				parent[input.size()]=j;
			}
		}

		//generate output tags
		stack<int> s;
		for(int d=input.size();d>0;d--){
			s.push(parent[d]);
		}
		while(!s.empty()){
			tagOutput.push_back(storage.indexTags[s.top()]);
			s.pop();
		}
		return;
	}

	void generateOutput(ofstream &outfile){
		for(int i=0;i<(int)input.size();i++){
			outfile << input[i] << "/" << tagOutput[i] << " ";
		}
		return;
	}
};

int main(int argc, char* argv[]){
	string testFilename = argv[1];
	string modelFilename = argv[2];
	string outFilename = argv[3];
	ifstream infile1, infile2;
	ofstream outfile;
	infile1.open(testFilename.c_str(),ios::in);
	if(infile1.fail()){
		return 0;
	}
	infile2.open(modelFilename.c_str(),ios::in);
	if(infile2.fail()){
		return 0;
	}
	outfile.open(outFilename.c_str(),ios::out);
	if(outfile.fail()){
		return 0;
	}

	run_tagger rt;
	rt.importData(infile2);
	rt.readInput(infile1);
	rt.viterbiAlgorithm();
	rt.generateOutput(outfile);
	
	return 0;
}
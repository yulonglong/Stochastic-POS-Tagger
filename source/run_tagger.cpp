//run_tagger.cpp
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


class run_tagger{
public:
	Storage storage;

	bool importData(string filename){
		return storage.importData(filename);
	}

	vector< vector<string> > inputSentences;
	vector< vector<string> > tagOutput;

	bool readInput(string filename){
		ifstream infile;
		infile.open(filename.c_str(),ios::in);
		if(infile.fail()){
			return false;
		}
		string line;
		string word;
		while(getline(infile,line)){
			vector<string> lineInput;
			istringstream ifstream(line);
			while(getline(ifstream,word,' ')){
				lineInput.push_back(word);
			}
			inputSentences.push_back(lineInput);
		}

		infile.close();
		return true;
	}

	void initViterbi(vector< vector<double> > &dp, vector< vector<int> > &parent, int inputSize) {
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

		initViterbi(dp, parent, inputSize);

		//first loop to initialize the first state
		//i.e. connecting the start node to the first state nodes.
		for(int i=0;i<SMALLTAGSIZE;i++){
			parent[0][i] = 45;
		}

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
						parent[d][i]=j;
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
};

int main(int argc, char* argv[]){
	string testFilename = argv[1];
	string modelFilename = argv[2];
	string outFilename = argv[3];

	run_tagger rt;

	cout << endl;
	cout << "Reading model file : " << modelFilename << endl;
	rt.importData(modelFilename);

	cout << endl;
	cout << "Reading input/test data from file : " << testFilename << endl;
	rt.readInput(testFilename);

	cout << endl;
	cout << "Processing Data (using Viterbi Algorithm)" << endl;
	rt.processData();

	cout << endl;
	cout << "Writing output to file : " << outFilename << endl;
	rt.printOutput(outFilename);

	cout << endl;
	cout << "Done! run_tagger is terminated." << endl;
	cout << endl;
	
	return 0;
}
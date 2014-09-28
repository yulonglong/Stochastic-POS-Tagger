//build_tagger.cpp
//@author Steven Kester Yuwono
//@matric A0080415N


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

//INIT DATA STRUCTURE
int transitionTagTable[TAGSIZE][TAGSIZE];
vector< vector<int> > wordTagTable;
int tagTable[TAGSIZE];
vector<int> wordTable;


double tagProbTable[TAGSIZE][TAGSIZE];
vector< vector<double> > wordTagProbTable;
int totalWordBag = 0;
int totalWordType = 0;
//END INIT DATA STRUCTURE

//TAGS MAPPING START
map<string,int> tags;
map<int,string> indexTags;

void setTags(){
	string pennTags[] = { "CC", "CD", "DT", "EX", "FW", "IN",
			"JJ", "JJR", "JJS", "LS", "MD", "NN", "NNS", "NNP", "NNPS", "PDT",
			"POS", "PRP", "PRP$", "RB", "RBR", "RBS", "RP", "SYM", "TO", "UH",
			"VB", "VBD", "VBG", "VBN", "VBP", "VBZ", "WDT", "WP", "WP$", "WRB",
			"$", "#", "``", "''", "-LRB-", "-RRB-", ",", ".", ":", "<s>",
			"</s>" };
	for(int i=0;i<TAGSIZE;i++){
		tags.insert(pair<string,int>(pennTags[i],i));
		indexTags.insert(pair<int,string>(i,pennTags[i]));
	}
	return;
}

int getTagIndex(string tag){
	map<string,int>::iterator it;
	it = tags.find(tag);
	if(it==tags.end()){
		return -1;
	}
	else{
		return it->second;
	}
}
//TAGS MAPPING END

//WORD MAPPNG STARTS
map<string,int> words;
map<int,string> indexWords;

int insertWord(string word){
	map<string,int>::iterator it;
	it = words.find(word);
	if(it==words.end()){
		int index = words.size();
		words.insert(pair<string,int>(word,index));
		indexWords.insert(pair<int,string>(index,word));
		vector<int> emptyVec (TAGSIZE,0);
		wordTagTable.push_back(emptyVec);
		wordTable.push_back(0);
		return index;
	}
	else{
		return it->second;
	}
};

int getWordIndex(string word){
	map<string,int>::iterator it;
	it = words.find(word);
	if(it==words.end()){
		return -1;
	}
	else{
		return it->second;
	}
}
//WORD MAPPING ENDS

void init(){
	setTags();
	memset(transitionTagTable,0,sizeof(transitionTagTable));
	wordTagTable.clear();
	vector<int> emptyVec (TAGSIZE,0);
	wordTagTable.push_back(emptyVec);
}

int getDelimiterIndex(string wordAndTag){
	int index = wordAndTag.length()-1;
	for(int i=index;i>=0;i--){
		if(wordAndTag[i]=='/'){
			return i;
		}
	}
	return -1;
}

void splitWordAndTag(string wordAndTag, string &word, string &tag){
	int delimIndex = getDelimiterIndex(wordAndTag);
	word = wordAndTag.substr(0,delimIndex);
	tag = wordAndTag.substr(delimIndex+1);
	return;
}


void importData(ifstream &infile){
	string temp;
	string tempString;

	getline(infile,temp,':');//ignore string before colon
	getline(infile,tempString);
	totalWordBag = atoi(tempString.c_str());
	getline(infile,temp,':');//ignore string before colon
	getline(infile,tempString);
	totalWordType = atoi(tempString.c_str());
	getline(infile,temp);//ignore one line

	for(int i=0;i<TAGSIZE;i++){
		infile >> temp;
		for(int j=0;j<TAGSIZE;j++){
			infile >> tagProbTable[i][j];
		}
	}
	getline(infile,temp);//ignore from cin
	getline(infile,temp);//ignore one line
	for(int i=0;i<totalWordType;i++){
		string word;
		infile >> word;
		insertWord(word);
		vector<double> emptyVec (TAGSIZE,0);
		wordTagProbTable.push_back(emptyVec);
		for(int j=0;j<TAGSIZE;j++){
			infile >> wordTagProbTable[i][j];
		}
	}
	return;
}

//for debugging purposes..
void exportData(ofstream &outfile){
	outfile << "Total Word Bag :" << totalWordBag << endl;
	outfile << "Total Word Type :" << totalWordType << endl;
	outfile << "Matrix of t(i-1) against t(i):" << endl;
	for(int i=0;i<TAGSIZE;i++){
		outfile << indexTags[i] << " ";
		for(int j=0;j<TAGSIZE;j++){
			outfile << tagProbTable[i][j] << " ";
		}
		outfile << endl;
	}
	outfile << "Matrix of t(i) against w(i):" << endl;
	for(int i=0;i<totalWordType;i++){
		outfile << indexWords[i] << " ";
		for(int j=0;j<TAGSIZE;j++){
			outfile << wordTagProbTable[i][j] << " ";
		}
		outfile << endl;
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
	int wordIndex = getWordIndex(input[0]);
	for(int i=0;i<TAGSIZE-2;i++){
		dp[0][i]=log2(tagProbTable[i][45])+log2(wordTagProbTable[wordIndex][i]);
	}

	//real DP
	for(int d=1;d<(int)input.size();d++){
		wordIndex = getWordIndex(input[d]);
		for(int i=0;i<TAGSIZE-2;i++){
			double maxPrev = -DBL_MAX;
			//get the maximum from the previous nodes
			for(int j=0;j<TAGSIZE-2;j++){
				double tempDouble = dp[d-1][j]+log2(tagProbTable[i][j]);
				//cout << tempDouble << endl;
				if(maxPrev<tempDouble){
					maxPrev = tempDouble;
					parent[d]=j;
				}
			}
			dp[d][i]=maxPrev+log2(wordTagProbTable[wordIndex][i]);
		}
	}

	double maxPrev = -DBL_MAX;
	for(int j=0;j<TAGSIZE-2;j++){
		double tempDouble = dp[input.size()-1][j]+log2(tagProbTable[46][j]);
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
		tagOutput.push_back(indexTags[s.top()]);
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

int main(int argc, char* argv[]){
	init();
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

	importData(infile2);

	readInput(infile1);
	
	viterbiAlgorithm();

	generateOutput(outfile);


	return 0;
}
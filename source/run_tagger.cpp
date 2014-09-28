//build_tagger.cpp
//@author Steven Kester Yuwono
//@matric A0080415N


#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#include <cstdlib>
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

int main(int argc, char* argv[]){
	init();
	string testFilename = argv[1];
	string modelFilename = argv[2];
	string outFilename = argv[3];
	ifstream infile;
	ofstream outfile;

	// infile.open(testFilename.c_str(),ios::in);
	// if(infile.fail()){
	// 	return 0;
	// }

	infile.open(modelFilename.c_str(),ios::in);
	if(infile.fail()){
		return 0;
	}

	//importData(infile);

	// outfile.open(outFilename.c_str(),ios::out);
	// if(outfile.fail()){
	// 	return 0;
	// }
	

	return 0;
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#define TAGSIZE 47
using namespace std;

map<string,int> tags;

void setTags(){
	string pennTags[] = { "CC", "CD", "DT", "EX", "FW", "IN",
			"JJ", "JJR", "JJS", "LS", "MD", "NN", "NNS", "NNP", "NNPS", "PDT",
			"POS", "PRP", "PRP$", "RB", "RBR", "RBS", "RP", "SYM", "TO", "UH",
			"VB", "VBD", "VBG", "VBN", "VBP", "VBZ", "WDT", "WP", "WP$", "WRB",
			"$", "#", "``", "''", "-LRB-", "-RRB-", ",", ".", ":", "<s>",
			"</s>" };
	for(int i=0;i<TAGSIZE;i++){
		tags.insert(pair<string,int>(pennTags[i],i));
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

map<string,int> words;
int tagTable[TAGSIZE][TAGSIZE];
vector< vector<int> > wordTagTable;

void insertWord(string word){
	map<string,int>::iterator it;
	it = words.find(word);
	if(it==words.end()){
		int index = words.size();
		words.insert(pair<string,int>(word,index));
		vector<int> emptyVec (TAGSIZE,0);
		wordTagTable.push_back(emptyVec);
	}
	return;
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


void init(){
	memset(tagTable,0,sizeof(tagTable));
	wordTagTable.clear();
	vector<int> emptyVec (TAGSIZE,0);
	wordTagTable.push_back(emptyVec);
}

void countData(ifstream &infile){
	string line;
	while(getline(infile,line)){
		string prevTag = "<s>";
		istringstream istream(line);
		string word, tag;
		while((getline(istream,word,'/'))&&(getline(istream,tag,' '))){
			int tagIndex = getTagIndex(tag);
			int prevTagIndex = getTagIndex(prevTag);
			tagTable[prevTagIndex][tagIndex]+=1;
			insertWord(word);
			int wordIndex = getWordIndex(word);
			wordTagTable[tagIndex][wordIndex]++;
		}
	}
}

int main(int argc, char* argv[]){
	string trainingFilename = argv[0];
	string devtFilename = argv[1];
	string modelFilename = argv[2];

	int totalWordBag = 0;
	int totalWordType = 0;

	return 0;
}
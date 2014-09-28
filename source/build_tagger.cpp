#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#define TAGSIZE 47
using namespace std;

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

map<string,int> words;
map<int,string> indexWords;
int tagTable[TAGSIZE][TAGSIZE];
vector< vector<int> > wordTagTable;
int totalWordBag = 0;
int totalWordType = 0;

void insertWord(string word){
	map<string,int>::iterator it;
	it = words.find(word);
	if(it==words.end()){
		int index = words.size();
		words.insert(pair<string,int>(word,index));
		indexWords.insert(pair<int,string>(index,word));
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
	setTags();
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
			tagTable[tagIndex][prevTagIndex]+=1;
			insertWord(word);
			int wordIndex = getWordIndex(word);
			wordTagTable[wordIndex][tagIndex]+=1;
			prevTag = tag;
			totalWordBag++;
		}
	}
	totalWordType = words.size();
	return;
}

void exportData(ofstream &outfile){
	outfile << "Total Word Bag :" << totalWordBag << endl;
	outfile << "Total Word Type :" << totalWordType << endl;
	outfile << "Matrix of t(i-1) against t(i):" << endl;
	for(int i=0;i<TAGSIZE;i++){
		outfile << indexTags[i] << " ";
		for(int j=0;j<TAGSIZE;j++){
			outfile << tagTable[i][j] << " ";
		}
		outfile << endl;
	}
	outfile << "Matrix of t(i) against w(i):" << endl;
	for(int i=0;i<totalWordType;i++){
		outfile << indexWords[i] << " ";
		for(int j=0;j<TAGSIZE;j++){
			outfile << wordTagTable[i][j] << " ";
		}
		outfile << endl;
	}
	return;
}

int main(int argc, char* argv[]){
	init();
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

	countData(infile);
	exportData(outfile);
	

	return 0;
}
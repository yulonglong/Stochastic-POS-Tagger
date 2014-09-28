//build_tagger.cpp
//@author Steven Kester Yuwono
//@matric A0080415N


#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
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

void countData(ifstream &infile){
	string line;
	while(getline(infile,line)){
		string prevTag = "<s>";
		istringstream istream(line);
		string word, tag;
		string wordAndTag;
		//general case, counting the occurences of t(i),t(i-1)
		while (getline(istream,wordAndTag,' ')){
			splitWordAndTag(wordAndTag,word,tag);
			//cout << word << " / " << tag << endl;
			int tagIndex = getTagIndex(tag);
			int prevTagIndex = getTagIndex(prevTag);
			transitionTagTable[tagIndex][prevTagIndex]+=1;
			tagTable[tagIndex] += 1;
			int wordIndex = insertWord(word);
			wordTagTable[wordIndex][tagIndex]+=1;
			wordTable[wordIndex]+=1;
			prevTag = tag;
			totalWordBag++;
		}
		//update for the sentence close tag
		tag = "</s>";
		int tagIndex = getTagIndex(tag);
		int prevTagIndex = getTagIndex(prevTag);
		transitionTagTable[tagIndex][prevTagIndex]+=1;
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



void addOneSmoothing(){
	//ADD ONE SMOOTHING
	for(int i=0;i<TAGSIZE;i++){
		for(int j=0;j<TAGSIZE;j++){
			transitionTagTable[i][j] += 1;
			tagTable[i]+=1;
			tagTable[j]+=1;
		}
	}
	for(int i=0;i<totalWordType;i++){
		for(int j=0;j<TAGSIZE;j++){
			wordTagTable[i][j] += 1;
			wordTable[i] += 1;
			tagTable[j] += 1;
		}
	}
	//END ADD ONE SMOOTHING
	for(int i=0;i<TAGSIZE;i++){
		for(int j=0;j<TAGSIZE;j++){
			tagProbTable[i][j] = (double)transitionTagTable[i][j]/(double)tagTable[j];
		}
	}
	for(int i=0;i<totalWordType;i++){
		for(int j=0;j<TAGSIZE;j++){
			wordTagProbTable[i][j] = (double)wordTagTable[i][j]/(double)tagTable[j];
		}
	}
	return;
}



void processData(){
	vector<double> emptyVec (TAGSIZE,0);
	for(int i=0;i<totalWordType;i++){
		wordTagProbTable.push_back(emptyVec);
	}

	addOneSmoothing();
	

	
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
	processData();
	exportData(outfile);
	

	return 0;
}
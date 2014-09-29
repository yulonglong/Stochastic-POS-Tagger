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


class Storage{
public:
	int transitionTagTable[TAGSIZE][TAGSIZE];
	vector< vector<int> > wordTagTable;
	int tagTable[TAGSIZE];
	vector<int> wordTable;


	double tagProbTable[TAGSIZE][TAGSIZE];
	vector< vector<double> > wordTagProbTable;
	int totalWordBag;
	int totalWordType;

	map<string,int> tags;
	map<int,string> indexTags;

	map<string,int> words;
	map<int,string> indexWords;



	Storage(){
		setTags();
		memset(transitionTagTable,0,sizeof(transitionTagTable));
		wordTagTable.clear();
		vector<int> emptyVec (TAGSIZE,0);
		wordTagTable.push_back(emptyVec);
		totalWordType = 0;
		totalWordBag = 0;
	}

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
};

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
				storage.transitionTagTable[tagIndex][prevTagIndex]+=1;
				storage.tagTable[tagIndex] += 1;
				int wordIndex = storage.insertWord(word);
				storage.wordTagTable[wordIndex][tagIndex]+=1;
				storage.wordTable[wordIndex]+=1;
				prevTag = tag;
				storage.totalWordBag++;
			}
			//update for the sentence close tag
			tag = "</s>";
			int tagIndex = storage.getTagIndex(tag);
			int prevTagIndex = storage.getTagIndex(prevTag);
			storage.transitionTagTable[tagIndex][prevTagIndex]+=1;
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
				storage.transitionTagTable[i][j] += 1;
				storage.tagTable[i]+=1;
				storage.tagTable[j]+=1;
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.wordTagTable[i][j] += 1;
				storage.wordTable[i] += 1;
				storage.tagTable[j] += 1;
			}
		}
		//END ADD ONE SMOOTHING
		for(int i=0;i<TAGSIZE;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.tagProbTable[i][j] = (double)storage.transitionTagTable[i][j]/(double)storage.tagTable[j];
			}
		}
		for(int i=0;i<storage.totalWordType;i++){
			for(int j=0;j<TAGSIZE;j++){
				storage.wordTagProbTable[i][j] = (double)storage.wordTagTable[i][j]/(double)storage.tagTable[j];
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
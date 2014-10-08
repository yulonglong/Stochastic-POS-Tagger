//Storage.cpp
//@author Steven Kester Yuwono
//@matric A0080415N

// to run the program, please have the following 3 cpp files in the same directory
// 1. Storage.cpp
// 2. build_tagger.cpp
// 3. run_tagger.cpp

// Please also place the training, development and test file in the same directory:
// 1. sents.train
// 2. sents.devt
// 3. sents.test

// compile build_tagger and run_tagger:
// $> g++ -Wall build_tagger.cpp -o build_tagger.exe
// $> g++ -Wall run_tagger.cp -o run_tagger.exe

// then run the program as described in the pdf
// $> build_tagger.exe sents.train sents.devt model_file
// $> run_tagger.exe sents.test model_file sents.out

// the answer will be in sents.out

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cstring>
#include <cstdlib>
#define TAGSIZE 47
using namespace std;

class Storage{
public:

	//initialize count table
	int transitionTagCountTable[TAGSIZE][TAGSIZE];
	vector< vector<int> > wordTagCountTable;
	int tagCountTable[TAGSIZE];
	vector<int> wordCountTable;

	//initialize probability table
	double tagProbTable[TAGSIZE][TAGSIZE];
	double tagUnigramProbTable[TAGSIZE];
	vector< vector<double> > wordTagProbTable;
	vector<double> wordTagUnigramProbTable;

	//initialize bag of words and word type count
	int totalWordBag;
	int totalWordType;

	//initialize maps for tags and words
	map<string,int> tags;
	map<int,string> indexTags;

	map<string,int> words;
	map<int,string> indexWords;


	//initialize the necessary attributes in the constructor
	Storage(){
		memset(tagProbTable,0,sizeof(tagProbTable));
		memset(tagUnigramProbTable,0,sizeof(tagUnigramProbTable));

		setTags();
		memset(transitionTagCountTable,0,sizeof(transitionTagCountTable));
		wordTagCountTable.clear();

		vector<int> emptyVec (TAGSIZE,0);
		wordTagCountTable.push_back(emptyVec);
		
		totalWordType = 0;
		totalWordBag = 0;

		//add one word "<UNK>" to handle unknown words.
		int wordIndex = insertWord("<UNK>");
		wordCountTable[wordIndex]+=1;
		totalWordBag+=1;
	}

	//initialize the tags map with Penn Treebank POS Tags
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

	//return tag index given a string of POS tag
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
	
	//insert a word into the word map
	int insertWord(string word){
		map<string,int>::iterator it;
		it = words.find(word);
		if(it==words.end()){
			int index = words.size();
			words.insert(pair<string,int>(word,index));
			indexWords.insert(pair<int,string>(index,word));
			vector<int> emptyVec (TAGSIZE,0);
			wordTagCountTable.push_back(emptyVec);
			wordCountTable.push_back(0);
			return index;
		}
		else{
			return it->second;
		}
	}


	//convert a string to all lowercase
	string stringToLower(string word){
		for(int i=0;i<(int)word.length();i++){
			word[i] = tolower(word[i]);
		}
		return word;
	}

	//convert the first char of a string to uppercase
	string stringFirstCharToUpper(string word){
		word[0] = toupper(word[0]);
		return word;
	}

	//try to find a word from the word map, and return its index
	//if it is not found, try to find the string with all lowercase characters
	//if it is still not found, try to find the string with first letter uppercase
	//if it is still not found, return the index of "<UNK>" (unknown word)
	int getWordIndex(string word){
		map<string,int>::iterator it;
		it = words.find(word);
		//if word is found, return index straight away
		if(it!=words.end()){
			return it->second;
		}

		//if word not found
		//try with the lowercase word
		string lowercaseWord = stringToLower(word);
		it = words.find(lowercaseWord);
		//if word is found, return index straight away
		if(it!=words.end()){
			return it->second;
		}

		//if word not found
		//try with the first letter Uppercase word
		string uppercaseWord = stringFirstCharToUpper(lowercaseWord);
		it = words.find(uppercaseWord);
		//if word is found, return index straight away
		if(it!=words.end()){
			return it->second;
		}

		//if still not found
		//return the index of "<UNK>" aka unknown words
		it = words.find("<UNK>");
		return it->second;
	}

	//get the index of the slash '/' which separates a word and its POS Tags
	int getDelimiterIndex(string wordAndTag){
		int index = wordAndTag.length()-1;
		for(int i=index;i>=0;i--){
			if(wordAndTag[i]=='/'){
				return i;
			}
		}
		return -1;
	}

	//split a word and a tag from a combined format of "word/tag"
	void splitWordAndTag(string wordAndTag, string &word, string &tag){
		int delimIndex = getDelimiterIndex(wordAndTag);
		word = wordAndTag.substr(0,delimIndex);
		tag = wordAndTag.substr(delimIndex+1);
		return;
	}

	//export the necessary attributes/tables to file (model_file).
	bool exportData(string filename){
		FILE* outfile;
		outfile = fopen(filename.c_str(),"w+");
		if (outfile == NULL){
			return false;
		}

		fprintf(outfile,"Total Word Bag :%d\n",totalWordBag);
		fprintf(outfile,"Total Word Type :%d\n",totalWordType);
		fprintf(outfile,"Matrix of t(i-1) against t(i):\n");
		for(int i=0;i<TAGSIZE;i++){
			fprintf(outfile,"%s ",indexTags[i].c_str());
			for(int j=0;j<TAGSIZE;j++){
				fprintf(outfile,"%.5e ",tagProbTable[i][j]);
			}
			fprintf(outfile,"\n");
		}
		fprintf(outfile,"Matrix of t(i) against w(i):\n");
		for(int i=0;i<totalWordType;i++){
			fprintf(outfile,"%s ",indexWords[i].c_str());
			for(int j=0;j<TAGSIZE;j++){
				fprintf(outfile, "%.5e ",wordTagProbTable[i][j]);
			}
			fprintf(outfile,"\n");
		}
		fclose(outfile);
		return true;
	}

	//import the necessary attributes/tables from file (model_file).
	bool importData(string filename){
		FILE* infile;
		infile = fopen(filename.c_str(),"r+");
		
		if (infile == NULL){
			return false;
		}

		fscanf(infile,"Total Word Bag :%d\n",&totalWordBag);
		fscanf(infile,"Total Word Type :%d\n",&totalWordType);
		fscanf(infile,"Matrix of t(i-1) against t(i):\n");
		for(int i=0;i<TAGSIZE;i++){
			char temp[10];
			fscanf(infile,"%s ",temp);
			for(int j=0;j<TAGSIZE;j++){
				fscanf(infile,"%lf ",&tagProbTable[i][j]);
			}
			fscanf(infile,"\n");
		}
		fscanf(infile,"Matrix of t(i) against w(i):\n");
		for(int i=0;i<totalWordType;i++){
			char buffer[1000];
			fscanf(infile,"%s ",buffer);
			string word = buffer;
			insertWord(word);
			vector<double> emptyVec (TAGSIZE,0);
			wordTagProbTable.push_back(emptyVec);
			for(int j=0;j<TAGSIZE;j++){
				fscanf(infile, "%lf ",&wordTagProbTable[i][j]);
			}
			fscanf(infile,"\n");
		}
		fclose(infile);
		return true;
	}
};
//Storage.cpp
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

class Storage{
public:
	int transitionTagCountTable[TAGSIZE][TAGSIZE];
	vector< vector<int> > wordTagCountTable;
	int tagCountTable[TAGSIZE];
	vector<int> wordCountTable;

	double tagProbTable[TAGSIZE][TAGSIZE];
	vector< vector<double> > wordTagProbTable;

	int totalWordBag;
	int totalWordType;

	map<string,int> tags;
	map<int,string> indexTags;

	map<string,int> words;
	map<int,string> indexWords;


	//initialize the necessary attributes in the constructor
	Storage(){
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
			wordTagCountTable.push_back(emptyVec);
			wordCountTable.push_back(0);
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
			//return the index of "<UNK>" aka unknown words
			it = words.find("<UNK>");
		}
		return it->second;
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
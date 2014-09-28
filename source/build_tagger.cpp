#include <iostream>
#include <map>
#define TAGSIZE 47
using namespace std;

map<string,int> tags;

void setPOSTags(){
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

int getPOSTagsIndex(string tag){
	map<string,int>::iterator it;
	it = tags.find(tag);
	if(it==tags.end()){
		return -1;
	}
	else{
		return it->second;
	}
}

int main(){
	return 0;
}
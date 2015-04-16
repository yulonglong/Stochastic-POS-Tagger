//split_n_fold.cpp
//@author Steven Kester Yuwono


// a program to split data, for n-fold cross-validation

#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

vector<string> sentences;

int main(){
	cout << "Enter training filename : ";
	string inputFilename;
	cin >> inputFilename;

	cout << "Enter output filename: ";
	string outputFilename;
	cin >> outputFilename;

	cout << "Enter the number of fold: ";
	int fold;
	cin >> fold;

	ifstream infile;

	infile.open(inputFilename.c_str(),ios::in);
	if(infile.fail()){
		return 0;
	}
	
	string sentence;
	while (getline(infile,sentence)){
		sentences.push_back(sentence);
	}

	for(int i=1;i<=fold;i++){
		cout << "Processing " << i << " out of " <<  fold << endl;
		int tenPercent = sentences.size()/fold;

		ostringstream oss1;
		oss1 << outputFilename << i << ".train";
		string trainFilename = oss1.str();

		ostringstream oss2;
		oss2 << outputFilename << i << ".test";
		string testFilename = oss2.str();

		FILE* outtrain;
		outtrain = fopen(trainFilename.c_str(),"w+");
		FILE* outtest;
		outtest = fopen(testFilename.c_str(),"w+");


		for(int j=0;j<(int)sentences.size();j++){
			if ((j>=tenPercent*(i-1))&&(j<tenPercent*i)){
				fprintf(outtest,"%s\n",sentences[j].c_str());
			}
			else{
				fprintf(outtrain,"%s\n",sentences[j].c_str());
			}
		}
		fclose(outtest);
		fclose(outtrain);
	}

	infile.close();

	cout << "Done!" << endl;
	
	return 0;
}
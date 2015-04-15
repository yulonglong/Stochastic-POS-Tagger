#include <iostream>
#include <cstdlib>
using namespace std;

int main () {
	int tc;
	cout << "Number of cases : "; cin >> tc; 
	double sumRecall = 0;
	double sumPrecision = 0;
	double sumF2Measure = 0;
	for(int d=1;d<=tc;d++){
		double TP,FP,TN,FN;
		cout << "TP : "; cin >> TP;
		cout << "FP : "; cin >> FP;
		cout << "TN : "; cin >> TN;
		cout << "FN : "; cin >> FN;
		double recall = TP/(TP+FN);
		double precision = TP/(TP+FP);
		double f2measure = (5.0*precision*recall)/((4.0*precision)+recall);

		cout << "Recall : " << recall << endl;
		cout << "Precision : " << precision << endl;
		cout << "F2Measure : " << f2measure << endl << endl;
		sumRecall += recall;
		sumPrecision += precision;
		sumF2Measure += f2measure;
	}
	cout << "Average Recall : " << sumRecall/(double)tc  << endl;
	cout << "Average Precision : " << sumPrecision/(double)tc << endl;
	cout << "Average F2Measure : " << sumF2Measure/(double)tc << endl;
	system("pause");
	return 0;
}
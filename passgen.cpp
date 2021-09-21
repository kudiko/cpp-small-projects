//============================================================================
// Name        : password_generator.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include "rng.h"
using namespace std;



int main() {
	cout << "Enter number of characters: " << endl;

	int N;
	cin >> N;
	int64_t *rng_nums=RNG_Upgrade(N);
	const string path = "output.txt";
		ofstream output(path);
	for (int i=0; i<N; ++i){
		cout << static_cast<char>(rng_nums[i]%94+33);
		output << static_cast<char>(rng_nums[i]%94+33);
	}
	return 0;
}

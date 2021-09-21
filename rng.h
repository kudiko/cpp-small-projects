/*
 * rng.h
 *
 *  Created on: Sep 17, 2021
 *      Author: dmitry
 */

#ifndef RNG_H
#define RNG_H
#include <vector>
using namespace std;

int64_t *RNG_Upgrade(int N);


void Cycle (){
	static int i = 0;
	++i;
	i%=4;
	cout << i << endl;
}

#endif /* RNG_H_ */

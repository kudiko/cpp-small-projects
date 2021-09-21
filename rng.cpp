/*
 * rng.cpp
 *
 *  Created on: Sep 17, 2021
 *      Author: dmitry
 */

#include <iostream>
#include <cstdint>
#include <fstream>
#include <string>
#include <ctime>
using namespace std;

template <typename array_type> void PrintArray( array_type *array, int N){
	for (int i=0; i<N; ++i){
		cout << array[i] << ' ';
		if(i==N-1){
			cout << endl;
		}
	}
}
int64_t *RNG(int64_t a, int64_t b, int64_t m, int64_t seed, int N, int max_bound){
	int64_t *nums = new int64_t[N];
	nums[0]=(seed*a+b)%m;
	for (int i=1; i<N; ++i){
		nums[i]=(nums[i-1]*a+b)%m;
	}
	for (int i=0; i<N; ++i){
		nums[i]%=max_bound;
	}
	return nums;
}
int64_t *RNG_Upgrade(int N){
	int64_t seed = time(0);
	int64_t max_bound=10'000'000'000'000'000;
	int64_t *array1=RNG(7141, 54773, 259200, seed, 2*N, max_bound);
	int64_t *array2=RNG(8121, 28411, 134456, seed, 2*N, max_bound);
	for(int i=0; i<2*N; ++i){
		array1[i] += (array2[i]/367);
	}
	int64_t *array3=RNG(8121, 28411, 134456, seed%1000, N, 2*N);
	int64_t *array4 = new int64_t[N];
	for(int i=0; i<N; ++i){
		array4[i]=array1[array3[i]];
	}
	delete[] array1;
	delete[] array2;
	delete[] array3;
	array1=array2=array3=nullptr;
	return array4;
}

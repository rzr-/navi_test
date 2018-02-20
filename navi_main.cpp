#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <stdlib.h>

//#include "navi_Barrier.h"
#include "navi_BarrierMap.h"

using namespace std;

int main () {
	BarrierMap m;
	m.parse ("data/config.xml", "data/barriers.xml");

	Barrier *b = m.getBarrier (2410, 607095); //Found
	b = m.getBarrier(2420, 607095); //Not Found
	if (b)
		cout << b->type.c_str() << endl;

	string filename = m.toString();
	vector <Barrier> resultBarriers = m.getBarriers (2410);
	printf ("%s\n", filename.c_str());
}

//g++ -c navi_main.cpp && g++ -c navi_BarrierMap.cpp && g++  navi_BarrierMap.o navi_main.o -o navi

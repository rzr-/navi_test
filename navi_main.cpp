#include <cstdio>
#include <vector>
#include <string>
#include <stdlib.h>

#include "navi_BarrierMap.h"

using namespace std;

int main () {
	BarrierMap *m = new BarrierMap();
	m->parse ("data/config.xml", "data/barriers.xml");

	Barrier *b = m->getBarrier (2410, 607095); //Found
	if (b)	printf ("Type of barrier: %s\n", b->type.c_str() );

	b = m->getBarrier(2420, 607095); //Not Found
	if (b)	printf ("%s\n", b->type.c_str() );

	vector <Barrier> resultBarriers = m->getBarriers (2410);
	
	printf ("%s\n", m->toString().c_str());

	resultBarriers.clear();
	delete m;
}

//g++ navi_main.cpp navi_BarrierMap.cpp -o navi && ./navi

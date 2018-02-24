#include <cstdio>
#include <vector>
#include <string>
#include <stdlib.h>

#include "navi_BarrierMap.h"

using namespace std;

void test_GetBarrier (Barrier *b) {
	
}


void test_GetBarriers () {

}
int main () {
	BarrierMap *m = new BarrierMap();
	m->parse ("data/config.xml", "data/barriers.xml");
	Barrier *b;
	vector <Barrier> resultBarriers;
	int i, j;
	/*
	test GetBarrier (int subloc_id, int barier_id)
	*/
	for (i=0; i<10; i++) {
		int barrier_id, subloc_id;
		scanf ("%d%d", &subloc_id, &barrier_id);
		b = m->getBarrier (subloc_id, barrier_id);

		b ? printf("Barrier found. Type:%s\n", b->type.c_str()) : printf("Barrier not found\n");
	}
	delete b;

	/*
	test getBarriers (int subloc_id);
	*/
	for (i=0; i<5; i++) {
		int subloc_id;

		scanf ("%d", &subloc_id);
		resultBarriers.clear();
		resultBarriers = m->getBarriers (subloc_id);

		if (!resultBarriers.empty()) {
			printf("Barriers in subllocation %d :\n", subloc_id);
			for (j=0; j<resultBarriers.size(); j++)
				printf ("%d ", resultBarriers[j].id);
		}
		else 
			printf("Barriers in sublocation %d not found", subloc_id);
		printf ("\n");
	}

	/*
	test getBarriers (int subloc_id, x, y);
	*/
	int subloc_id;
	double x, y;
	scanf ("%d", &subloc_id);
	for (i=0; i<7; i++) {
		scanf ("%lf %lf", &x, &y);
		resultBarriers.clear();

		resultBarriers = m->getBarriers (subloc_id, x, y);

		if (!resultBarriers.empty()) {
			for (j=0; j<resultBarriers.size(); j++)
				printf ("Point %.13lf, %.13lf inside barrier(s):%d\n", x, y, resultBarriers[j].id);
		}
		else 
			printf("Point %.13lf, %.13lf not in barrier\n", x, y);
	}

	/* 
	testintersects (int subloc_id, double x1, double y1, double x2, double y2);
	*/
	scanf ("%d", &subloc_id);
	double x1, y1, x2, y2;
	for (i=0; i<2; i++) {
		scanf ("%lf %lf %lf %lf", &x1, &y1, &x2, &y2);
		x = m->intersects (subloc_id, x1, y1, x2, y2);
	}

	resultBarriers.clear();
	printf ("%s\n", m->toString().c_str());
	delete m;
}

//g++ navi_main.cpp navi_BarrierMap.cpp -o navi && ./navi < tests/tests > tests/results

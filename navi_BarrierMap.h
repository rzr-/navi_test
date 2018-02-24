#include <vector>
//#include <string>

#include "navi_SubLoc.h"
#include "navi_Barrier.h"

class BarrierMap {
	public:
		std::vector <Barrier> barriers;
		std::vector <SubLoc> sublocs;

		int comp (double a, double b);
		bool IsOnLine (double x1, double y1, double x2, double y2, double x, double y);
		std::pair <double, double> inter (double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

		std::pair <double, double> getSublocWH (int subloc_id);
		void parse (const char* configXml, const char* barriersXml);
		Barrier *getBarrier (int subloc_id, int barrier_id);
		std::vector <Barrier> getBarriers (int subloc_id);
		std::vector <Barrier> getBarriers (int subloc_id, double x, double y);
		double intersects (int subloc_id, double x1, double y1, double x2, double y2);
		std::string toString()const;
};
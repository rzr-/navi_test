#include <vector>
//#include <string>

#include "navi_SubLoc.h"
#include "navi_Barrier.h"

class BarrierMap {
	public:
		std::vector <Barrier> barriers;
		std::vector <SubLoc> sublocs;

		void parse (const char* configXml, const char* barriersXml);
		Barrier *getBarrier (int subloc_id, int barrier_id);
		std::vector <Barrier> getBarriers (int subloc_id);
		std::vector <Barrier> getBarriers (int subloc_id, double x, double y);
		double intersects (int subloc_id, double x1, double y1, double x2, double y2);
		std::string toString()const;
};
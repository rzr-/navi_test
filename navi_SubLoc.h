#include <string>

class SubLoc {
	public:
		int location_id;
		int id;
		std::string name;
		double width;
		double height;

		SubLoc (int loc_id, int subloc_id, std::string n, double w, double h):
			location_id (loc_id),
			id (subloc_id),
			name (n),
			width (w),
			height (h) { }
};
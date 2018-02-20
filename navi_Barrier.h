#include <vector>
//#include <string>

struct coords {
	double x;
	double y;
};

class Barrier {
	public:
		int subloc_id;
		std::string type;
		int id;
		std::vector <coords> coords_x;
		std::vector <coords> coords_kx;
};
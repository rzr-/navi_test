#include "navi_BarrierMap.h"
#include "libs/tinyxml/tinyxml.h"
#include "libs/tinyxml/tinyxml.cpp"
#include "libs/tinyxml/tinystr.h"
#include "libs/tinyxml/tinystr.cpp"
#include "libs/tinyxml/tinyxmlerror.cpp"
#include "libs/tinyxml/tinyxmlparser.cpp"

#include <cmath>

using namespace std;

/*******************
Parsing data from config.xml, barriers.xml
to vector <Barrier> barriers, and 
   vector <SubLoc> sublocs

Args: configXml (char*) - path to config.xml
	  barriersXml (char*) - patth to barriers.xml
*******************/
void BarrierMap::parse (const char* configXml, const char* barriersXml) {
	//parse config
	TiXmlDocument doc(configXml);
	if (doc.LoadFile()) {
		TiXmlHandle hDoc(&doc);
		TiXmlElement *pRoot, *pParm;
		pRoot = doc.FirstChildElement("Location");
		if(pRoot) {
			pParm = pRoot->FirstChildElement("Sublocation");
			while(pParm) {
				SubLoc subloc (
						atoi ( pRoot->Attribute("id") ),
						atoi ( pParm->Attribute("id") ),
							   pParm->Attribute("name"),
						atof ( pParm->FirstChildElement("Metrics")->Attribute("width_meters") ),
						atof ( pParm->FirstChildElement("Metrics")->Attribute("height_meters") )
				);
				sublocs.push_back (subloc);
				pParm = pParm->NextSiblingElement("Sublocation");
			}
		}
	}
	else
		printf("Failed to load file \"%s\"\n", configXml);

	//parse barriers
	TiXmlDocument doc2(barriersXml);
	if (doc2.LoadFile())
	{
		TiXmlHandle hDoc(&doc2);
		TiXmlElement *pRoot, *pSublocations, *pBarriers, *pCoords;
		pRoot = doc2.FirstChildElement("Location");
		if(pRoot) {
			pSublocations = pRoot->FirstChildElement("Sublocation");
			while(pSublocations) {
				Barrier b;
				b.subloc_id = atoi (pSublocations->Attribute("id"));
				pBarriers = pSublocations->FirstChildElement("Barrier");
				while (pBarriers) {
					b.id = atoi (pBarriers->Attribute("id"));
					b.type = pBarriers->Attribute("type");
					pCoords = pBarriers->FirstChildElement("coord");
					while(pCoords) {
						coords c;
						pCoords->Attribute("x", &c.x);
						pCoords->Attribute("y", &c.y);
						b.coords_x.push_back(c);
						pCoords->Attribute("kx", &c.x);
						pCoords->Attribute("ky", &c.y);
						b.coords_kx.push_back(c);
						pCoords = pCoords->NextSiblingElement("coord");
					}
					barriers.push_back(b);
					b.coords_x.clear();
					b.coords_kx.clear();
					pBarriers = pBarriers->NextSiblingElement("Barrier");
				}
				pSublocations = pSublocations->NextSiblingElement("Sublocation");
			}
		}
	}
	else
		printf("Failed to load file \"%s\"\n", barriersXml);
}

/*******************
Search for barrier with given sublocation and barrier ids
Args: subloc_id (int) - id of the sublocation
	  barrier_id (int) - id of the barrier
Returns: pointer to vector element of type Barrier.
*******************/
Barrier *BarrierMap::getBarrier (int subloc_id, int barrier_id) {
	int size = barriers.size();
	
	for (int i=0; i<size; i++)
		if (barriers[i].subloc_id == subloc_id && barriers[i].id == barrier_id) {
			return &barriers[i];
		}

	return NULL;
}

/*******************
Search for barriers within given sublocation
Args: subloc_id (int) - id of the sublocation
Returns: vector of type Barrier with found barriers

Лучше возрващать указатель, так лишняя копия вектора получается
*******************/
vector <Barrier> BarrierMap::getBarriers (int subloc_id) {
	vector <Barrier> resultBarriers;
	int size = barriers.size(), i;
	int sublocs_size = sublocs.size(), k = 0;

	for (i=0; i<sublocs_size; i++)
		if (subloc_id == sublocs[i].id) { k = 1; break; }
	
	if (!k) { printf ("Zone does not exist.\n"); }
	else 
		for (i=0; i<size; i++)
			if (barriers[i].subloc_id == subloc_id)
				resultBarriers.push_back (barriers[i]);
	return resultBarriers;
}

/*******************
Returns:
	1 : a < b
	0 : a == b
   -1 : a > b
*******************/
int BarrierMap::comp (double a, double b) {
	int precision = 100000000;
	int _a = (int) (a * precision);
	int _b = (int) (b * precision);
	if (_a < _b)  return 1;
	if (_a == _b) return 0;
	if (_a >  _b) return -1;
}

pair <double, double> BarrierMap::getSublocWH (int subloc_id) {
	int size = sublocs.size();
	
	for (int i=0; i<size; i++)
		if (sublocs[i].id == subloc_id)
			return make_pair (sublocs[i].width, sublocs[i].height);

	return make_pair (-1, -1);
}

/*******************
Search for barriers which include given point within a sublocation

Using Crossing Number methond which counts the number of times a ray 
starting from the point P crosses the polygon boundary edges. 
The point is outside when this "crossing number" is even; 
otherwise, when it is odd, the point is inside.

Args: subloc_id (int) - id of the sublocation
	  x (double) - x coordinate of the point
	  y (double) - y coordinate of the point
Returns: vector of type Barrier with found barriers

Лучше возрващать указатель, так лишняя копия вектора получается
*******************/
vector <Barrier> BarrierMap::getBarriers (int subloc_id, double x, double y) {
	vector <Barrier> resultBarriers;
	vector <Barrier> _barriers = getBarriers(subloc_id);
	int size = _barriers.size();

	pair <double, double> sublocWH = getSublocWH(subloc_id);
	x = x/sublocWH.first;  //kx = x/width
	y = y/sublocWH.second; //ky = y/height
	
	
	for (int k=0; k<size; k++) {
		//is inside
		bool result = false;
		int j = _barriers[k].coords_kx.size() - 1;
		
		// ray casting method	
		for (int i = 0; i < _barriers[k].coords_kx.size(); i++) {
			double p = _barriers[k].coords_kx[i].x + (y - _barriers[k].coords_kx[i].y) / (_barriers[k].coords_kx[j].y - _barriers[k].coords_kx[i].y) * (_barriers[k].coords_kx[j].x - _barriers[k].coords_kx[i].x);
			bool comp1 = (comp(_barriers[k].coords_kx[i].y, y) == 1),
				 comp2 = (comp(_barriers[k].coords_kx[j].y, y) != 1),
				 comp3 = (comp(_barriers[k].coords_kx[j].y, y) == 1),
				 comp4 = (comp(_barriers[k].coords_kx[i].y, y) != 1),
				 comp5 = (comp(p, x) == 1);
			if ( ((comp1 && comp2) || (comp3 && comp4)) && (comp5) ) result = !result;
			j = i;
		}
		if (result)
			resultBarriers.push_back (_barriers[k]);
	}
	_barriers.clear();
	return resultBarriers;
}

bool BarrierMap::IsOnLine(double x1, double y1, double x2, double y2, double x, double y)
{
    return ((comp(x, x1) != 1) && (comp(x, x2) != -1)  || 
    	    (comp(x, x2) != 1) && (comp(x, x1) != -1)) && 
          (((comp(y, y1) != 1) && (comp(y, y2) != -1)) || 
           ((comp(y, y2) != 1) && (comp(y, y1) != -1))); 
}

pair <double, double> BarrierMap::inter (double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
		//equations of the form x=c (two vertical lines)
		if ((comp(x1, x2) == 0) && (comp(x3, x4) == 0) &&(comp(x1, x3) == 0))
			{
				printf ("Both lines overlap vertically, ambiguous intersection points.\n");
				return make_pair(-1, -1);
			}

			//equations of the form y=c (two horizontal lines)
			if ((comp(y1, y2) == 0) &&(comp(y3, y4) == 0) &&(comp(y1, y3) == 0) )
			{
				printf ("Both lines overlap horizontally, ambiguous intersection points.\n");
				return make_pair(-1, -1);
			}

			//equations of the form x=c (two vertical lines)
			if ((comp(x1, x2) == 0) && (comp(x3, x4) == 0))
			{
				//return default(Point);
				return make_pair(-1, -1);
			}

			//equations of the form y=c (two horizontal lines)
			if ((comp(y1, y2) == 0) &&(comp(y3, y4) == 0))
			{
				//return default(Point);
				return make_pair(-1, -1);
			}

			//general equation of line is y = mx + c where m is the slope
			//assume equation of line 1 as y1 = m1x1 + c1 
			//=> -m1x1 + y1 = c1 ----(1)
			//assume equation of line 2 as y2 = m2x2 + c2
			//=> -m2x2 + y2 = c2 -----(2)
			//if line 1 and 2 intersect then x1=x2=x & y1=y2=y where (x,y) is the intersection point
			//so we will get below two equations 
			//-m1x + y = c1 --------(3)
			//-m2x + y = c2 --------(4)

			double x, y;

			//lineA is vertical x1 = x2
			//slope will be infinity
			//so lets derive another solution
			if ((comp(x1, x2) == 0))
			{
				//compute slope of line 2 (m2) and c2
				double m2 = (y4 - y3) / (x4 - x3);
				double c2 = -m2 * x3 + y3;

				//equation of vertical line is x = c
				//if line 1 and 2 intersect then x1=c1=x
				//subsitute x=x1 in (4) => -m2x1 + y = c2
				// => y = c2 + m2x1 
				x = x1;
				y = c2 + m2 * x1;
			}
			//lineB is vertical x3 = x4
			//slope will be infinity
			//so lets derive another solution
			else if ((comp(x3, x4) == 0) )
			{
				//compute slope of line 1 (m1) and c2
				double m1 = (y2 - y1) / (x2 - x1);
				double c1 = -m1 * x1 + y1;

				//equation of vertical line is x = c
				//if line 1 and 2 intersect then x3=c3=x
				//subsitute x=x3 in (3) => -m1x3 + y = c1
				// => y = c1 + m1x3 
				x = x3;
				y = c1 + m1 * x3;
			}
			//lineA & lineB are not vertical 
			//(could be horizontal we can handle it with slope = 0)
			else
			{
				//compute slope of line 1 (m1) and c2
				double m1 = (y2 - y1) / (x2 - x1);
				double c1 = -m1 * x1 + y1;

				//compute slope of line 2 (m2) and c2
				double m2 = (y4 - y3) / (x4 - x3);
				double c2 = -m2 * x3 + y3;

				//solving equations (3) & (4) => x = (c1-c2)/(m2-m1)
				//plugging x value in equation (4) => y = c2 + m2 * x
				x = (c1 - c2) / (m2 - m1);
				y = c2 + m2 * x;

				//verify by plugging intersection point (x, y)
				//in orginal equations (1) & (2) to see if they intersect
				//otherwise x,y values will not be finite and will fail this check
				if (!(-m1 * x + y == c1
					&& -m2 * x + y == c2))
				{
					//return default(Point);
					return make_pair(-1, -1);
				}
			}

			//x,y can intersect outside the line segment since line is infinitely long
			//so finally check if x, y is within both the line segments
			if (IsOnLine(x1, y1, x2, y2, x, y) &&
				IsOnLine(x3, y3, x4, y4, x, y))
			{
				//return new Point() { x = x, y = y };
				return make_pair(x, y);
				//printf ("1 Intersection at point %lf %lf\n", x, y);
			}

			//return default null (no intersection)
			//return default(Point);
			return make_pair(-1, -1);

}

/*******************
Search for barriers which intersect with given line within a sublocation

Using Crossing Number methond which counts the number of times a ray 
starting from the point P crosses the polygon boundary edges. 
The point is outside when this "crossing number" is even; 
otherwise, when it is odd, the point is inside.

Args: subloc_id (int) - id of the sublocation
	  x1 (double) - Ax coordinate of the line
	  y1 (double) - Ay coordinate of the line
	  x2 (double) - Bx coordinate of the line
	  y2 (double) - By coordinate of the line

Returns:
	NAN - subLoc не найден
	-0 - точка (x1,y1) лежит внутри барьера
	-1 - отрезок (x1,y1) - (x2,y2) не пересекает барьер;
	-coef [0,1] - отрезок (x1,y1) - (x2,y2) пересекает барьер в точке (x1 + k*(x2-x1), y1 + k*(y2-y1))
	
	prints "NAN" if subloc not  found
	prints intersection point of line and barrier 
	prints if point (x1, y1) inside banner

**********/
double BarrierMap::intersects (int subloc_id, double x1, double y1, double x2, double y2) {
	
	bool nan = true;

	pair <double, double> sublocWH = getSublocWH(subloc_id);
	x1 = x1/sublocWH.first;  //kx = x/width
	y1 = y1/sublocWH.second; //ky = y/height
	x2 = x2/sublocWH.first;  //kx = x/width
	y2 = y2/sublocWH.second; //ky = y/height

	for (int i=0; i<sublocs.size(); i++)   //check if Sublocation
		if (sublocs[i].id == subloc_id) {  //with id == subloc_id exist
			nan = false; 				   //if not -> return NAN
			break;
		}

	if (nan) {
		printf ("NAN");
		return -1;
	}

	int cn = 0;
	bool inside;
	double coef = -1;
	vector <Barrier> _barriers = getBarriers(subloc_id);
	int size = _barriers.size();	

	double x3, y3, x4, y4;
	double A1, B1, C1, A2, B2, C2;
	pair <double, double> intersection;

	for (int k=0; k<size; k++) {
		inside = false;
		coef = -1;
			// loop through all edges of the polygon
		for (int i=0; i<_barriers[k].coords_kx.size()-1; i++) { // edge from barriers[i] to barriers[i+1]
			x3 = _barriers[k].coords_kx[i].x,
			y3 = _barriers[k].coords_kx[i].y,
			x4 = _barriers[k].coords_kx[i+1].x,
			y4 = _barriers[k].coords_kx[i+1].y;

			intersection = inter (x1, y1, x2, y2, x3, y3, x4, y4);
			if (intersection.first != -1 && intersection.second != -1) {
				printf ("Intersection of line %.8lf %.8lf %.8lf %.8lf ", x1, y1, x2, y2); 
				printf ("with line %.8lf %.8lf %.8lf %.8lf ", x3, y3, x4, y4); 
				printf ("at point %.8lf %.8lf\n", intersection.first, intersection.second); 
				break;
			}		
		}
	}

	for (int k=0; k<size; k++) {
		//is inside
		bool result = false;
		int j = _barriers[k].coords_kx.size() - 1;
			
		for (int i = 0; i < _barriers[k].coords_kx.size(); i++) {
			// <  ->  1
			// == ->  0
			// >  -> -1

			// ray casting method
			double p = _barriers[k].coords_kx[i].x + (y1 - _barriers[k].coords_kx[i].y) / (_barriers[k].coords_kx[j].y - _barriers[k].coords_kx[i].y) * (_barriers[k].coords_kx[j].x - _barriers[k].coords_kx[i].x);
			bool comp1 = (comp(_barriers[k].coords_kx[i].y, y1) == 1),
				 comp2 = (comp(_barriers[k].coords_kx[j].y, y1) != 1),
				 comp3 = (comp(_barriers[k].coords_kx[j].y, y1) == 1),
				 comp4 = (comp(_barriers[k].coords_kx[i].y, y1) != 1),
				 comp5 = (comp(p, x1) == 1);
			if ( ((comp1 && comp2) || (comp3 && comp4)) && (comp5) ) result = !result;
			j = i;
		}
		if (result) {//inside = true;
			printf ("Point %.8lf %8lf inside barrier id: %d", x1, y1, _barriers[k].id);
		}
	}	
	return -1;
}

string BarrierMap::toString() const {
	TiXmlDocument doc;  

	TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "utf-8", "" );  
	doc.LinkEndChild( decl );  

	TiXmlElement *root = new TiXmlElement( "Location" ); 
	root->SetAttribute("id", sublocs[0].location_id);
	doc.LinkEndChild( root ); 

	int i, j, k;

	for (i=0; i<sublocs.size(); i++) {
		TiXmlElement *Sublocation = new TiXmlElement( "Sublocation" ); 
		Sublocation->SetAttribute("id", sublocs[i].id);
		
		for (j=0; j<barriers.size(); j++) {
			if (barriers[j].subloc_id == sublocs[i].id ) {
				TiXmlElement *Barrier = new TiXmlElement( "Barrier" ); 
				Barrier->SetAttribute("type", barriers[j].type.c_str() );
				Barrier->SetAttribute("id", barriers[j].id);
				
				for(k=0; k<barriers[j].coords_kx.size(); k++) {
					TiXmlElement *coord = new TiXmlElement( "coord" ); 
					coord->SetDoubleAttribute("x", barriers[j].coords_x[k].x);
					coord->SetDoubleAttribute("y", barriers[j].coords_x[k].y);
					coord->SetDoubleAttribute("kx", barriers[j].coords_kx[k].x);
					coord->SetDoubleAttribute("ky", barriers[j].coords_kx[k].y);
					Barrier->LinkEndChild( coord );
				}
				Sublocation->LinkEndChild( Barrier );
			}
		}
		root->LinkEndChild( Sublocation );
	}
	doc.SaveFile( "data/new_barriers.xml" );

	TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	doc.Accept( &printer );
	//fprintf( stdout, "%s", printer.CStr() );
	return printer.CStr();
}

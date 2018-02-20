#include "navi_BarrierMap.h"
#include "libs/tinyxml/tinyxml.h"
#include "libs/tinyxml/tinyxml.cpp"
#include "libs/tinyxml/tinystr.h"
#include "libs/tinyxml/tinystr.cpp"
#include "libs/tinyxml/tinyxmlerror.cpp"
#include "libs/tinyxml/tinyxmlparser.cpp"

using namespace std;

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

Barrier *BarrierMap::getBarrier (int subloc_id, int barrier_id) {
	int size = barriers.size();
	
	for (int i=0; i<size; i++)
		if (barriers[i].subloc_id == subloc_id && barriers[i].id == barrier_id) {
			printf ("Found\n");
			return &barriers[i];
		}

	printf ("Not found\n");
	return NULL;
}

/***************
Лучше возрващать указатель, так лишняя копия вектора получается
***************/
vector <Barrier> BarrierMap::getBarriers (int subloc_id) {
	vector <Barrier> resultBarriers;
	int size = barriers.size();
	
	for (int i=0; i<size; i++)
		if (barriers[i].subloc_id == subloc_id) 
			resultBarriers.push_back (barriers[i]);
	return resultBarriers;
}

vector <Barrier> BarrierMap::getBarriers(int subloc_id, double x, double y) {
	vector <Barrier> resultBarriers;
	int cn = 0; // the  crossing number counter
	int size = barriers.size();
	/***************
		x, y -> kx, ky !!!			
	***************/
	for (int k=0; k<size; k++) {
		if (barriers[k].subloc_id == subloc_id) {
			// loop through all edges of the polygon
			for (int i=0; i<barriers[k].coords_kx.size()-1; i++)   // edge from barriers.coord_kx[i] to barriers.coord_kx[i+1]
				if (
				   ( (barriers[k].coords_kx[i].y <= y) && (barriers[k].coords_kx[i+1].y > y) ) || // an upward crossing
				   ( (barriers[k].coords_kx[i].y > y)  && (barriers[k].coords_kx[i+1].y <=  y) )  // a downward crossing
				   ) { 
					// compute  the actual edge-ray intersect x-coordinate
					double vt = (double) (y  - barriers[k].coords_kx[i].y) / (barriers[k].coords_kx[i+1].y - barriers[k].coords_kx[i].y);
					if (x <  barriers[k].coords_kx[i].x + vt * (barriers[k].coords_kx[i+1].x - barriers[k].coords_kx[i].x)) // x < intersect
						 ++cn;   // a valid crossing of y=y right of x
				}
			if (cn&1)     // 0 if even (out), and 1 if odd (in)
				resultBarriers.push_back (barriers[k]);
		}
	}
	return resultBarriers;
}

/**********
Returns:
	NAN - subLoc не найден
	0 - точка (x1,y1) лежит внутри барьера
	1 - отрезок (x1,y1) - (x2,y2) не пересекает барьер;
	coef [0,1] - отрезок (x1,y1) - (x2,y2) пересекает барьер в точке (x1 + k*(x2-x1), y1 + k*(y2-y1))
**********/
double BarrierMap::intersects(int subloc_id, double x1, double y1, double x2, double y2) {
	bool nan = true;

	for (int i=0; i<sublocs.size(); i++)
		if (sublocs[i].id == subloc_id) {
			nan = false; 
			break;
		}
	if (nan) {
		printf ("NAN");
		return -1;
	}

	int cn = 0;
	bool inside = false;
	double coef = -1;
	double m1, c1, m2, c2;
	double _x1, _y1, _x2, _y2;
	double dx, dy, _dx, _dy;
	double intersection_X, intersection_Y;
	int size = barriers.size();

	/***************	
	x, y -> kx, ky !!!  
	***************/

	dx = x2 - x1;
	dy = y2 - y1;
	m1 = dy / dx;
	// y = mx + c
	// intercept c = y - mx
	c1 = y1 - m1 * x1; // which is same as y2 - slope * x2
	
	for (int k=0; k<size; k++) {
		if (barriers[k].subloc_id == subloc_id) {
			// loop through all edges of the polygon
			for (int i=0; i<barriers[k].coords_kx.size()-1; i++) { // edge from barriers[i]  to barriers[i+1]
				_x1 = barriers[k].coords_kx[i].x,
				_y1 = barriers[k].coords_kx[i].y,
				_x2 = barriers[k].coords_kx[i+1].x,
				_y2 = barriers[k].coords_kx[i+1].y;
				
				_dx = _x2 - _x1;
				_dy = _y2 - _y1;
			 
				m2 = _dy / _dx;
				c2 = _y2 - m2 * _x2; 
				//if( (m1 - m2) == 0)
				//	printf ("No intersection\n");
				if (m1 - m2 != 0) {
					intersection_X = (c2 - c1) / (m1 - m2);
					intersection_Y = m1 * intersection_X + c1;
					coef = (intersection_X - x1) / (x2-x1);  // k
					break;
				}

				// inside or not
				if (
				   ( (barriers[k].coords_kx[i].y <= y1) && (barriers[k].coords_kx[i+1].y > y1) ) ||
				   ( (barriers[k].coords_kx[i].y > y1)  && (barriers[k].coords_kx[i+1].y <=  y1) ) 
				   ) { 

					double vt = (double) (y1  - barriers[k].coords_kx[i].y) / (barriers[k].coords_kx[i+1].y - barriers[k].coords_kx[i].y);
					if (x1 <  barriers[k].coords_kx[i].x + vt * (barriers[k].coords_kx[i+1].x - barriers[k].coords_kx[i].x)) 
						 ++cn; 
				}
			}
			if (cn&1) inside = true;
		}
	}

	if (coef == -1) {
		printf ("No intersection");
		return 1;
	}
	else {
		printf ("intersection at point %lf\n", coef);
		printf ("Point (x1, y1) inside barrier: %s", inside ? "true" : "false");
	}
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

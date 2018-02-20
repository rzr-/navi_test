/** zones.h
 *
 * Author: Fedor Puchkov <fedormex@gmail.com>
 * Copyright (c) 2016 Navigine. All rights reserved.
 *
 */

#ifndef _NAVIGINE_ZONES_H_
#define _NAVIGINE_ZONES_H_

#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdio.h>

class LocationPoint;

class Zone
{
  public:
    int                           id;
    std::string                   name;
    std::string                   color;
    std::string                   alias;
    std::vector<LocationPoint>    points;
    int                           timeout;
    
  public:
    inline Zone(int id = 0)
      : id      ( id )
      , timeout ( 0  )
    { }
    
    inline Zone(const Zone& z)
      : id      ( z.id      )
      , name    ( z.name    )
      , color   ( z.color   )
      , alias   ( z.alias   )
      , points  ( z.points  )
      , timeout ( z.timeout )
    { }
    
    inline bool isValid(void) const { return id > 0; }
    inline bool operator==(const Zone& z)const { return id == z.id; }
    inline bool operator< (const Zone& z)const { return id <  z.id; }
    
    bool containsPoint(double x, double y)const;
};

class ZoneMap
{
  public:
    struct SubLocation
    {
      int             id;     // Identifier
      double          width;  // Width in meters
      double          height; // Height in meters
      std::set<Zone>  zones;  // Zones on the sublocation
      
      inline SubLocation()
        : id     ( 0 )
        , width  ( 0.0 )
        , height ( 0.0 )
        , zones  ( )
      { }
      
      inline SubLocation(const SubLocation& subLoc)
        : id     ( subLoc.id )
        , width  ( subLoc.width  )
        , height ( subLoc.height )
        , zones  ( subLoc.zones  )
      { }
      
      inline SubLocation(int _id, double _w = 0.0, double _h = 0.0)
        : id     ( _id )
        , width  ( _w  )
        , height ( _h  )
        , zones  ( )
      { }
      
      inline bool isValid()const { return id > 0; }
      inline bool operator==(const SubLocation& subLoc)const { return id == subLoc.id; }
      inline bool operator< (const SubLocation& subLoc)const { return id <  subLoc.id; }
    };
    
  public:
    ZoneMap();
    ZoneMap(const ZoneMap& zoneMap);
    
    // Clear/empty graph
    void clear();
    bool isEmpty()const;
    
    // Set/get location
    void setLocation(int id);
    int getLocation()const;
    
    // Add/get sublocation
    void addSubLocation(const SubLocation& subLoc);
    SubLocation getSubLocation(int id)const;
    
    Zone getZone(int subLoc, int id)const;
    std::vector<Zone> getZones(const LocationPoint& P)const;
    std::vector<Zone> getZones(int subLoc, double x, double y)const;
    std::vector<Zone> getZones(int subLoc)const;
    
    static ZoneMap parse(const std::string& configXml,
                         const std::string& zonesXml,
                         std::string* error = 0,
                         bool* ok = 0);
    
    std::string toString()const;
    void print(FILE* fp = stdout)const;
    
  private:
    bool parseConfig(const std::string& configXml, std::string* error = 0);
    bool parseZones (const std::string& zonesXml,  std::string* error = 0);
    
  private:
    int mLocation;
    std::set<SubLocation> mSubLocations;
};

extern bool PointOnLine        ( double x, double y, double x1, double y1, double x2, double y2 );
extern bool XRayIntersectsLine ( double x, double y, double x1, double y1, double x2, double y2 );

#endif

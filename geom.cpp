#include "geom.hpp"
#include <cmath>
#include <algorithm>
#include <limits>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <memory>
#include "lib/doctest.h"
#include "lib/miniball.hpp"

static const double DEG_TO_RAD = M_PI/180.0;
static const double RAD_TO_DEG = 180.0/M_PI;

namespace complib {
  namespace bg = boost::geometry;


// // To find orientation of ordered triplet (p, q, r).
// // The function returns following values
// // 0 --> p, q and r are colinear
// // 1 --> Clockwise
// // 2 --> Counterclockwise
// int FindOrientation(
//   const Point2D &p,
//   const Point2D &q,
//   const Point2D &r
// ){
//   const int val = (q.y-p.y) * (r.x-q.x) - (q.x-p.x) * (r.y-q.y);

//   if (val == 0) 
//     return 0;   //Colinear
//   else if (val>0)
//     return 1;   //Clockwise
//   else
//     return 2;   //Counter-clockwise
// }


double RingArea(const Polygon::ring_type &r){
  return std::abs(bg::area(r));
}

double RingPerim(const Polygon::ring_type &r){
  return std::abs(bg::perimeter(r));
}

double areaHoles(const Polygon &p){
  return std::abs(std::accumulate(
    p.inners().begin(),
    p.inners().end(),
    0.0,
    [](const double b, const Polygon::ring_type &r){ return b+RingArea(r);}
  ));
}

double areaHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+areaHoles(p);}
  ); 
}

double areaIncludingHoles(const Polygon &p){
  return RingArea(p.outer()) + std::accumulate(
    p.inners().begin(),
    p.inners().end(),
    0.0,
    [](const double b, const Polygon::ring_type &r){ return b+RingArea(r);}
  );
}

double areaIncludingHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+areaIncludingHoles(p);}
  ); 
}

double areaExcludingHoles(const MultiPolygon &mp){
  return bg::area(mp.g); // areaIncludingHoles(mp)-areaHoles(mp);
}



double perimHoles(const Polygon &p){
  return std::accumulate(
    p.inners().begin(),
    p.inners().end(),
    0.0,
    [](const double b, const Polygon::ring_type &r){ return b+RingPerim(r);}
  );
}

double perimExcludingHoles(const Polygon &p){
  return RingPerim(p.outer());
}

double perimIncludingHoles(const Polygon &p){
  return perimExcludingHoles(p) + std::accumulate(
    p.inners().begin(),
    p.inners().end(),
    0.0,
    [](const double b, const Polygon::ring_type &r){ return b+RingPerim(r);}
  );
}

double perimExcludingHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+perimExcludingHoles(p);}
  );
}

double perimIncludingHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+perimIncludingHoles(p);}
  ); 
}

double perimHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+perimHoles(p);}
  ); 
}



double hullArea(const Polygon::ring_type &r){
  auto hull = GeomConvexHull(r);
  return bg::area(hull);
}

double hullAreaOuter(const Polygon &p){
  auto hull = GeomConvexHull(p);
  return bg::area(hull);
}

double hullAreaPolygonOuterRings(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+hullAreaOuter(p);}
  );
}

double hullAreaOfHoles(const Polygon &p){
  return std::accumulate(
    p.inners().begin(),
    p.inners().end(),
    0.0,
    [](const double b, const Polygon::ring_type &r){ return b+hullArea(r);}
  );
}

double hullAreaOfHoles(const MultiPolygon &mp){
  return std::accumulate(
    mp.g.begin(),
    mp.g.end(),
    0.0,
    [](const double b, const Polygon &p){ return b+hullAreaOfHoles(p);}
  );
}

double diameter(const Polygon::ring_type &ring){
  //TODO: There's a faster way to do this
  double maxdist = 0;
  for(unsigned int i=0;i<ring.size();i++)
  for(unsigned int j=i+1;j<ring.size();j++)
    maxdist = std::max(maxdist,bg::distance(ring.at(i),ring.at(j)));
  return maxdist;
}

double diameterOuter(const Polygon &p){
  return diameter(GeomConvexHull(p).outer());
}

double diameterOfEntireMultiPolygon(const MultiPolygon &mp){
  return diameter(GeomConvexHull(mp.g).outer());
}

}

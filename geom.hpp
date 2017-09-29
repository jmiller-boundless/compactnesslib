#ifndef _geom_hpp_
#define _geom_hpp_

#include <vector>
#include <cmath>
#include <set>
#include <string>
#include <limits>
#include "Props.hpp"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace complib {

namespace bg = boost::geometry;

void PrintProps(const Props &ps);

typedef boost::geometry::model::d2::point_xy<int> Point2D;
typedef boost::geometry::model::d2::point_xy<int> SimplePoint2D;
typedef boost::geometry::model::polygon<Point2D> Polygon;
typedef boost::geometry::model::polygon<SimplePoint2D> SimplePolygon;
typedef boost::geometry::model::multi_polygon<Polygon> SimpleMultiPolygon;
typedef boost::geometry::model::box<Point2D> Box;

//TODO
// boost::geometry::envelope(polygon, box);

class MultiPolygon {
 public:
  SimpleMultiPolygon g;
  Props props;
  Scores scores;
  void toRadians();
  void toDegrees();
  MultiPolygon intersect(const MultiPolygon &b) const;
  void reverse();

  std::set<unsigned int> neighbours;
  typedef std::pair<unsigned int, double> parent_t;
  std::vector<parent_t> parents;
  std::vector<parent_t> children;
};

typedef std::vector<MultiPolygon> MultiPolygons;

class GeoCollection {
 public:
  MultiPolygons g;
  std::string prj_str;
  void reverse();
  void correctWindingDirection();
};



//double EuclideanDistance(const Point2D &a, const Point2D &b);

double areaIncludingHoles(const Polygon &p);
double areaIncludingHoles(const MultiPolygon &mp);
double areaExcludingHoles(const MultiPolygon &mp);

double areaHoles(const Polygon &p);
double areaHoles(const MultiPolygon &mp);

double perimExcludingHoles(const Polygon &p);
double perimExcludingHoles(const MultiPolygon &mp);
double perimIncludingHoles(const Polygon &p);
double perimIncludingHoles(const MultiPolygon &mp);

double perimHoles(const Polygon &p);
double perimHoles(const MultiPolygon &mp);

double hullAreaOfOuter(const Polygon &p);
double hullAreaPolygonOuterRings(const MultiPolygon &mp);

double hullAreaOfHoles(const Polygon &p);
double hullAreaOfHoles(const MultiPolygon &mp);

//double diameter(const Ring &r);
double diameterOuter(const Polygon &p);
double diameterOfEntireMultiPolygon(const MultiPolygon &mp);



template<class T>
unsigned holeCount(const T &geom){
  return bg::num_interior_rings(geom);
}

template<class T>
unsigned polyCount(const T &geom){
  return bg::num_geometries(geom);
}



template<class T>
std::pair<Point2D, Point2D> MostDistantPoints(const T &geom){
  //We'll use the Convex Hull to find the two most distant points

  SimplePolygon hullpoly;
  boost::geometry::convex_hull(geom, hullpoly);

  std::pair<unsigned int, unsigned int> idx_maxpts;
  double maxdist = 0;

  const auto hull = hullpoly.outer();

  //TODO: There's a faster way to do this  
  for(unsigned int i=0;i<hull.size();i++)
  for(unsigned int j=i+1;j<hull.size();j++){
    const double dist = boost::geometry::distance(hull.at(i),hull.at(j));
    if(dist>maxdist){
      idx_maxpts = std::make_pair(i,j);
      maxdist    = dist;
    }
  }

  return std::make_pair(hull.at(idx_maxpts.first), hull.at(idx_maxpts.second));
}

template<class T, class U>
double IntersectionArea(const T &a, const U &b){
  SimpleMultiPolygon output;
  boost::geometry::intersection(a, b, output);

  return boost::geometry::area(output);
}

template<class T>
SimplePolygon GeomConvexHull(const T &geom){
  SimplePolygon hullpoly;
  boost::geometry::convex_hull(geom, hullpoly);
  return hullpoly;
}



/*
template<class T>
MultiPolygon GetBoundingCircle(const T &geom){
  //Number of unique points from which to construct the circle. The circle will
  //have one more point of than this in order to form a closed ring).
  const int CIRCLE_PT_COUNT = 1000;




  std::vector< std::vector<double> > pts;
  for(const auto &poly: mp)
  for(const auto &ring: poly)
  for(const auto &pt: ring)
    pts.push_back(std::vector<double>({{pt.x,pt.y}}));

  // define the types of iterators through the points and their coordinates
  // ----------------------------------------------------------------------
  typedef std::vector<std::vector<double> >::const_iterator PointIterator; 
  typedef std::vector<double>::const_iterator CoordIterator;

  // create an instance of Miniball
  // ------------------------------
  typedef Miniball::
    Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> > 
    MB;

  MB mb (2, pts.begin(), pts.end());
  
  const Point2D midpt(mb.center()[0], mb.center()[1]);
  const double radius = std::sqrt(mb.squared_radius());
  //"Computation time was "<< mb.get_time() << " seconds\n";

  MultiPolygon circle;
  circle.emplace_back();             //Make a polygon
  circle.back().emplace_back();      //Make a ring
  auto &ring = circle.back().back(); //Get the ring

  //Make a "circle"
  for(int i=0;i<CIRCLE_PT_COUNT;i++)
    ring.emplace_back(
      midpt.x+radius*std::cos(-2*M_PI*i/(double)CIRCLE_PT_COUNT),
      midpt.y+radius*std::sin(-2*M_PI*i/(double)CIRCLE_PT_COUNT)
    );
  //Close the "circle"
  ring.push_back(ring.front());

  return circle;
}*/



template<class T>
SimpleMultiPolygon GetBoundingCircleMostDistant(const T &geom){
  //Number of unique points from which to construct the circle. The circle will
  //have one more point of than this in order to form a closed ring).
  const int CIRCLE_PT_COUNT = 1000;

  const auto dist_pts = MostDistantPoints(geom);

  const Point2D &mpa = dist_pts.first;
  const Point2D &mpb = dist_pts.second;

  const Point2D midpt( 
    (bg::get<0>(mpa)+bg::get<0>(mpb))/2. ,
    (bg::get<1>(mpa)+bg::get<1>(mpb))/2. );
  const auto radius = boost::geometry::distance(mpa,mpb)/2.;

  SimpleMultiPolygon circle;

  //Make a "circle"
  for(int i=0;i<CIRCLE_PT_COUNT;i++)
    boost::geometry::append(circle, 
      Point2D(
        bg::get<0>(midpt)+radius*std::cos(-2*M_PI*i/(double)CIRCLE_PT_COUNT),
        bg::get<1>(midpt)+radius*std::sin(-2*M_PI*i/(double)CIRCLE_PT_COUNT)
      )
    );
  //Close the "circle"
  boost::geometry::append(circle, 
    Point2D(
      bg::get<0>(midpt)+radius*std::cos(0),
      bg::get<1>(midpt)+radius*std::sin(0)
    )
  );

  return circle;
}


template<class T>
SimpleMultiPolygon Buffer(const T &geom, const double pad_amount){
  const int points_per_circle = 36;
  boost::geometry::strategy::buffer::distance_symmetric<double> distance_strategy(pad_amount);
  static const boost::geometry::strategy::buffer::join_round    join_strategy  (points_per_circle);
  static const boost::geometry::strategy::buffer::end_round     end_strategy   (points_per_circle);
  static const boost::geometry::strategy::buffer::point_circle  circle_strategy(points_per_circle);
  static const boost::geometry::strategy::buffer::side_straight side_strategy;

  SimpleMultiPolygon buffered;
  boost::geometry::buffer(geom,buffered,distance_strategy,side_strategy,join_strategy,end_strategy,circle_strategy);

  return buffered;
}


template<class T>
SimpleMultiPolygon GetRingFromGeom(const T &geom, const double pad_amount){
  const auto grown  = Buffer(geom,  pad_amount);
  const auto shrunk = Buffer(geom, -pad_amount);

  SimpleMultiPolygon ring;
  boost::geometry::difference(grown, shrunk, ring);

  return ring;
}

}

#endif

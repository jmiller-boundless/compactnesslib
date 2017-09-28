#include "wkt.hpp"
#include "geom.hpp"
#include <fstream>
#include <streambuf>
#include <sstream>
#include <string>
#include <boost/geometry.hpp>

namespace complib {

GeoCollection ReadWKT(std::string wktstr){
  GeoCollection gc;
  gc.g.emplace_back(); //Make collection one long

  boost::geometry::read_wkt(wktstr, gc.g.back().g);
  
  return gc;
}

GeoCollection ReadWKTFile(std::string filename) {
  std::ifstream fin(filename);

  std::string wktstr((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

  return ReadWKT(wktstr);
}



std::string GetWKT(const MultiPolygon &mp){
  std::ostringstream ss;
  ss<<boost::geometry::wkt(mp.g);
  return ss.str();
}

}
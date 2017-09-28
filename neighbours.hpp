#ifndef _neighbours_hpp_
#define _neighbours_hpp_

#include "geom.hpp"

namespace complib {
  void FindNeighbouringDistricts(GeoCollection &gc, const double eps);
  void FindParents(GeoCollection &subunits, const GeoCollection &superunits, const double eps, const double cutoff);
}

#endif

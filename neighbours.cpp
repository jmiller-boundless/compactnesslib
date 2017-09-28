#include "neighbours.hpp"
#include "geom.hpp"
#include "SpIndex.hpp"
#include <vector>



//////////////////////////////////////////////////////////////////
//Okay, back to my stuff


namespace complib {

void FindNeighbouringDistricts(GeoCollection &gc, const double eps){
  SpIndex<int> sp;

  for(unsigned int mpi=0;mpi<gc.g.size();mpi++){
    Box bb;
    boost::geometry::envelope(gc.g.at(mpi).g, bb);
    Box bb_epsilon(
      Point2D(bb.min_corner().get<0>()-eps, bb.min_corner().get<1>()-eps),
      Point2D(bb.max_corner().get<0>()+eps, bb.max_corner().get<1>()+eps)
    );

    sp.add(bb_epsilon, mpi);
  }
  sp.buildIndex();


  for(auto i=boost::geometry::index::begin(sp.rtree); i!=boost::geometry::index::end(sp.rtree);i++){
    const auto possible_neighbours = sp.query(i->first);

    for(const auto &pn: possible_neighbours){
      const auto dist = boost::geometry::distance(gc.g.at(i->second).g, gc.g.at(pn).g);
      if(dist<=eps){
        gc.g.at(i->second).neighbours.insert(pn);
        gc.g.at(pn).neighbours.insert(i->second);
      }
    }
  }
}



void FindParents(GeoCollection &subunits, const GeoCollection &superunits, const double eps, const double cutoff){
  SpIndex<int> supidx;

  //Build an r-tree of the super units
  for(unsigned int mpi=0;mpi<superunits.g.size();mpi++)
    supidx.addDeferred(superunits.g.at(mpi).g, mpi);
  supidx.buildIndex();

  //Loop through the little units and find which superunits they overlap
  #pragma omp parallel for
  for(unsigned int mpi=0;mpi<subunits.g.size();mpi++){
    auto &subunit = subunits.g.at(mpi);

    subunit.props["EXTCHILD"] = "F";

    Box bb;
    boost::geometry::envelope(subunit.g, bb);
    // Box bb_epsilon(
    //   Point2D(bb.min_corner().get<0>()-eps, bb.min_corner().get<1>()-eps),
    //   Point2D(bb.max_corner().get<0>()+eps, bb.max_corner().get<1>()+eps)
    // );

    const auto potential_parents = supidx.query(bb);

    for(const auto &pp: potential_parents){
      //const auto   buffered = Buffer(subunit.g, eps);
      const double area     = boost::geometry::area(subunit.g);
      const double iarea    = IntersectionArea(subunit.g, superunits.g.at(pp).g);
      if(cutoff*area<=iarea && iarea<1){
        subunit.props["EXTCHILD"] = "T";
        subunit.parents.emplace_back(pp, iarea/area);
      } else if(iarea==1){
        subunit.parents.emplace_back(pp, 1.0);
        break;
      }
    }
  }  
}

}

#include "bounded_scores.hpp"
#include "geom.hpp"
#include <cmath>
#include <vector>
#include <stdexcept>
#include <unordered_map>

namespace complib {

double ScoreConvexHullPTB(const MultiPolygon &mp, const MultiPolygon &border){
  const double area      = areaIncludingHoles(mp);
  const double hull_area = IntersectionArea(GeomConvexHull(mp.g), border.g);

  double ratio = area/hull_area;
  if(ratio>1)
    ratio = 1;
  return ratio;
}



double ScoreReockPTB(const MultiPolygon &mp, const MultiPolygon &border){
  return 1; //TODO

  // const auto   circle = GetBoundingCircle(mp);
  // const auto   iarea  = IntersectionArea(circle, border.g);
  // const double area   = areaIncludingHoles(mp);

  // double ratio = area/iarea;
  // if(ratio>1)
  //   ratio = 1;
  // return ratio;
}



double ScoreBorderAreaUncertainty(const MultiPolygon &mp, const MultiPolygon &border){
  static int ringnum=0;
  ringnum++;
  //Amount by which we will grow the subunit
  const int pad_amount = 1000; //metres

  //If we've already determined the subunit is not an exterior child, then no
  //further calculation is necessary.
  if(mp.props.count("EXTCHILD") && mp.props.at("EXTCHILD")=="F")
    return 0;

  const auto mp_ring = GetRingFromGeom(mp.g,     pad_amount);
  const auto bo_ring = GetRingFromGeom(border.g, pad_amount);

  //Get the intersection of the border rings - uncertainty can only occur here
  SimpleMultiPolygon ring_intersection;
  boost::geometry::intersection(mp_ring, bo_ring, ring_intersection);


  
  //TODO
  //XOR the subunit and the superunit. This gives us the border uncertainty, but
  //also the entire rest of the superunit!
  // cl::Paths xored;
  // {
  //   cl::Clipper clpr;
  //   clpr.AddPaths(paths_mp, cl::ptSubject, true);
  //   clpr.AddPaths(paths_bo, cl::ptClip, true);
  //   cl::Paths solution;
  //   clpr.Execute(cl::ctXor, xored, cl::pftEvenOdd, cl::pftEvenOdd);
  // }

  //std::cerr<<ringnum<<",xored,\""<<OutputPaths(xored)<<"\""<<std::endl;

  //Get the intersection of the border ring and the xored area - this is an upper
  //bound on the uncertain area
  // cl::Paths isect;
  // {
  //   cl::Clipper clpr;
  //   clpr.AddPaths(ring_intersection, cl::ptSubject, true);
  //   clpr.AddPaths(xored, cl::ptClip, true);
  //   cl::Paths solution;
  //   clpr.Execute(cl::ctIntersection, isect, cl::pftEvenOdd, cl::pftEvenOdd);
  // }

  //std::cerr<<ringnum<<",isect,\""<<OutputPaths(isect)<<"\""<<std::endl;

  // double area = 0;
  // for(const auto &path: isect)
  //   area += cl::Area(path);

  // return area;

  return -9999;
}



void CalculateAllBoundedScores(
  GeoCollection &subunits,
  const GeoCollection &superunits,
  const std::string join_on
){
  CalculateListOfBoundedScores(subunits, superunits, join_on, getListOfBoundedScores());
}



void CalculateListOfBoundedScores(
  GeoCollection &subunits,
  const GeoCollection &superunits,
  const std::string join_on,
  std::vector<std::string> score_list
){
  if(score_list.empty())
    score_list = getListOfBoundedScores();
  else if(score_list.size()==1 && score_list.at(0)=="all")
    score_list = getListOfBoundedScores();


  if(join_on.empty() || superunits.g.size()==1){

    for(auto& sub: subunits.g){
      for(const auto &sn: score_list){
        if(bounded_score_map.count(sn))
          sub.scores[sn] = bounded_score_map.at(sn)(sub,superunits.g.at(0));
      }
    }

  } else {

    for(const auto &mp: subunits.g)
      if(!mp.props.count(join_on))
        throw std::runtime_error("At least one subunit was missing the joining attribute!");

    //A quick was to access superunits based on their key
    std::unordered_map<std::string, const MultiPolygon *> su_key;
    for(const auto &mp: superunits.g){
      if(!mp.props.count(join_on))
        throw std::runtime_error("At least one superunit was missing the joining attribute!");
      if(su_key.count(mp.props.at(join_on)))
        throw std::runtime_error("More than one superunit had the same key!");
      su_key[mp.props.at(join_on)] = &mp;
    }

    for(auto& sub: subunits.g){
      for(const auto &sn: score_list){
        if(bounded_score_map.count(sn))
          sub.scores[sn] = bounded_score_map.at(sn)(sub,*su_key.at(sub.props.at(join_on)));
      }
    }

  }
}



const std::vector<std::string>& getListOfBoundedScores(){
  static std::vector<std::string> score_names;
  if(!score_names.empty())
    return score_names;
  for(const auto &kv: bounded_score_map)
    score_names.push_back(kv.first);
  return score_names;
}



const bounded_score_map_t bounded_score_map({
  {"CvxHullPTB", ScoreConvexHullPTB},
  {"ReockPTB",   ScoreReockPTB},
  {"AreaUncert", ScoreBorderAreaUncertainty}
});

}

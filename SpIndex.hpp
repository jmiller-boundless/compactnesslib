#ifndef _SpIndex_hpp_
#define _SpIndex_hpp_

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "geom.hpp"

namespace complib {

template<class ValueType>
class SpIndex {
 public:
  typedef std::pair<Box, ValueType> value;

 private:
  typedef boost::geometry::index::rtree< value, boost::geometry::index::rstar<16> > rtree_t;
  std::vector<value> boxes_to_insert;

 public:
  rtree_t rtree;

  void add(const Box &bb, const ValueType id);
  void addDeferred(const Box &bb, const ValueType id);
  template<class T> void addDeferred(const T &geom, const int id);
  std::vector<ValueType> query(const Point2D &xy) const;
  template<class T> std::vector<ValueType> query(const T &bb) const;
  std::vector<ValueType> query(const Box &bb) const;
  void buildIndex();
};

template<class ValueType>
void SpIndex<ValueType>::add(
  const Box &bb,
  const ValueType id
){
  rtree.insert(std::make_pair(bb, id));
}

template<class ValueType>
void SpIndex<ValueType>::addDeferred(
  const Box &bb, 
  const ValueType id
){
  boxes_to_insert.push_back(std::make_pair(bb, id));
}


template<class ValueType>
template<class T>
void SpIndex<ValueType>::addDeferred(const T &geom, const int id){
  Box bb;
  boost::geometry::envelope(geom, bb);
  addDeferred(bb, id);
}


template<class ValueType>
void SpIndex<ValueType>::buildIndex(){
  rtree = rtree_t(boxes_to_insert);
  boxes_to_insert.clear();
  boxes_to_insert.shrink_to_fit();
}

template<class ValueType>
template<class T>
std::vector<ValueType> SpIndex<ValueType>::query(const T &geom) const {
  Box query_box;
  boost::geometry::envelope(geom, query_box);
  return query(query_box);
}

template<class ValueType>
std::vector<ValueType> SpIndex<ValueType>::query(const Box &qbo) const {
  std::vector<value> result_s;
  rtree.query(
    boost::geometry::index::intersects(qbo),
    std::back_inserter(result_s)
  );

  std::vector<ValueType> ret;
  for(const auto &x: result_s)
    ret.emplace_back(x.second);

  return ret;
}

}



#endif


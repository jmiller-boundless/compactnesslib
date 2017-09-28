#include "lib/json.hpp"
#include "geojson.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <streambuf>
#include <stdexcept>
#include <sstream>
#include <string>
#include <map>

using json = nlohmann::json;

namespace complib {

std::map<std::string, GeoCollection> prepped_geojson;

//Use to deduce types
//template<typename T> struct TD;
//e.g. TD<decltype(WHAT_AM_I_VAR_NAME)> td;

Polygon::ring_type ParseRing(const json &r){
  Polygon::ring_type temp;
  for(const auto &c: r)
    temp.emplace_back(c[0],c[1]);
  return temp;
}


//rj::StringBuffer sb;
//rj::Writer<rj::StringBuffer> writer(sb);
//d["properties"].Accept(writer);
//std::string s = sb.GetString();

Polygon ParsePolygon(const json &coor){
  Polygon poly;
  //First ring is the outer ring, all the others are holes
  for(unsigned int i=0;i<coor.size();i++){
    auto ring = ParseRing(coor[i]);
    if(i==0)
      poly.outer() = ring;
    else
      poly.inners().push_back(ring);
  }

  return poly;
}

const json& GetToCoordinates(const json &d){
  if(d.count("geometry")){
    return d.at("geometry").at("coordinates");
  } else if(d.count("coordinates")){
    return d.at("coordinates");
  } else {
    throw std::runtime_error("Could find neither geometry nor coordinates!");
  }
}

MultiPolygon ParseTopPolygon(const json &d){
  MultiPolygon mp;
  mp.g.push_back(ParsePolygon(GetToCoordinates(d)));
  return mp;
}

MultiPolygon ParseMultiPolygon(const json &d){
  MultiPolygon mp;
  for(const auto &poly: GetToCoordinates(d))
    mp.g.emplace_back(ParsePolygon(poly));

  return mp;
}

MultiPolygon ParseFeature(const json &d){
  MultiPolygon mp;

  const std::string geotype = d["geometry"]["type"];
  if(geotype=="MultiPolygon")
    mp = ParseMultiPolygon(d);
  else if(geotype=="Polygon")
    mp = ParseTopPolygon(d);
  else
    throw std::runtime_error("Unexpected data type - skipping!");


  if(d.count("properties")){
    const json &this_props = d["properties"];
    for(json::const_iterator it = this_props.begin(); it != this_props.end(); ++it){
      const json &thisval = this_props[it.key()];
      mp.props[it.key()] = thisval.dump();
    }
  }

  return mp;
}

GeoCollection ReadGeoJSON(const std::string geojson){
  if(geojson.compare(0,2,"__")==0)
    return prepped_geojson.at(geojson);

  GeoCollection gc;
  auto d = json::parse(geojson);

  if(!d.is_object())
    throw std::runtime_error("GeoJSON not an object!");

  if(!d.count("type"))
    throw std::runtime_error("No type property!");
  if(!d["type"].is_string())
    throw std::runtime_error("Type not a string!");

  if(d["type"]=="MultiPolygon"){
    gc.g.push_back(ParseMultiPolygon(d));
  } else if(d["type"]=="Polygon"){
    gc.g.push_back(ParseTopPolygon(d));
  } else if(d["type"]=="FeatureCollection"){
    for(const auto &f: d["features"])
      gc.g.push_back(ParseFeature(f));
  } else {
    throw std::runtime_error("Not a FeatureCollection or MultiPolygon or Polygon!");
  }

//  gc.correctWindingDirection();

  return gc;
}

GeoCollection ReadGeoJSONFile(std::string filename){
  std::ifstream fin(filename);

  std::string geojson((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

  return ReadGeoJSON(geojson);
}

std::string OutScoreJSON(const GeoCollection &gc, const std::string id){
  std::ostringstream oss;

  const bool use_id = !id.empty();

  oss<<"{\n";
  for(unsigned int i=0;i<gc.g.size();i++){
    oss<<"\t\"";
    if(use_id)
      oss<<gc.g.at(i).props.at(id);
    else
      oss<<i;
    oss<<"\":{\n";

    unsigned int inserted = 0;
    for(const auto &kv: gc.g.at(i).scores){
      oss<<"\t\t\""<<kv.first<<"\":"<<std::fixed<<std::setprecision(5)<<kv.second;
      if(inserted++<gc.g.at(i).scores.size()-1)
        oss<<",\n";
    }

    oss<<"\n\t}";
    if(i<gc.g.size()-1)
      oss<<",\n";
  }
  oss<<"\n}";

  return oss.str();
}

std::string PrepGeoJSON(std::string geojson){
  const std::string key = "__" + std::to_string(prepped_geojson.size());
  prepped_geojson[key] = ReadGeoJSON(geojson);
  return key;
}

}
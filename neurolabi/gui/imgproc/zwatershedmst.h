#ifndef ZWATERSHEDMST_H
#define ZWATERSHEDMST_H

#include <queue>
#include <vector>
#include <map>
#include <iostream>
#include "zstack.hxx"


using std::vector;
using std::priority_queue;
using std::map;


class ZEdge{
public:
  ZEdge(int from, int to, double weight):m_from(from),m_to(to),m_weight(weight){}
  bool operator < (const ZEdge& e)const{return m_weight < e.m_weight;}
public:
  int  m_from;
  int m_to;
  double  m_weight;
};


class ZActiveSet{
public:
  ZActiveSet(){m_label = 0;}
  ~ZActiveSet(){m_vertices.clear();}
  inline int getLabel()const{return m_label;}
  inline void setLabel(int label){m_label = label;}
  inline void add(int v){m_vertices.push_back(v);}
  inline void merge(const ZActiveSet& set){m_vertices.insert(m_vertices.end(),set.m_vertices.begin(),set.m_vertices.end());}
  inline uint size()const{return m_vertices.size();}
  inline vector<int>& data(){return m_vertices;}
private:
  int m_label;
  std::vector<int> m_vertices;
};


class ZWatershedMST{
public:
  ZWatershedMST(){}
  ~ZWatershedMST(){}

public:
  //1, ..., vertices are node labels
  void run(vector<int>& rv, int vertices, const vector<ZEdge>& edges, const map<int,int>& seeds);

};
#endif // ZWATERSHEDMST_H

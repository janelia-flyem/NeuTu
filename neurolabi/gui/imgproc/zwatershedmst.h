#ifndef ZWATERSHEDMST_H
#define ZWATERSHEDMST_H

#include <queue>
#include <vector>
#include <map>
#include <iostream>
#include "zstack.hxx"

class ZEdge
{
public:
  ZEdge(uint index_a, uint index_b, double weight){m_a = index_a, m_b = index_b, m_weight = weight;}
  bool operator < (const ZEdge& e)const{return m_weight < e.m_weight;}
public:
  uint m_a;
  uint m_b;
  double  m_weight;
};



class ZActiveSet
{
public:
  ZActiveSet(){m_label = 0;}
  ~ZActiveSet(){/*m_vertices.clear();*/}
  void setLabel(uint64_t label){m_label = label;}
  void add(uint v){m_vertices.push_back(v);}
  void merge(ZActiveSet* set){m_vertices.insert(m_vertices.end(),set->m_vertices.begin(),set->m_vertices.end());}
public:
  uint64_t m_label;
  std::vector<uint> m_vertices;
};


class ZWatershedMST
{
public:
  ZWatershedMST(ZStack* stack, ZStack* seed, double alpha, double beta){m_stack = stack; m_seed = seed;m_alpha = alpha; m_beta = beta;}
  ~ZWatershedMST(){/*m_map_of_activeset.clear();*/}

  ZStack* run();

private:
  void init();
  void updateActiveSet(ZActiveSet* source, ZActiveSet* target);
  inline double weight(uint8 a, uint8 b){return -std::exp(-m_alpha*(a+b)*(a+b))+std::exp(-m_beta*(a-b)*(a-b));}

private:
  ZStack* m_stack;
  ZStack* m_seed;
  double m_alpha, m_beta;
  std::priority_queue<ZEdge> m_queue_of_edges;
  std::vector<ZActiveSet*> m_map_of_activeset;
};


#endif // ZWATERSHEDMST_H

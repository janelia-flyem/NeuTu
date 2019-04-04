#include<memory>
#include<unordered_map>
#include "zwatershedmst.h"

using std::shared_ptr;


void ZWatershedMST::run(vector<int>& rv, const vector<int>& vertices,const vector<ZEdge>& edges,
const map<int,int>& seeds){
  std::priority_queue<ZEdge> que(edges.begin(),edges.end());
  std::unordered_map<int,shared_ptr<ZActiveSet>> activeset;

  for(uint i = 0; i < vertices.size(); ++i){
    shared_ptr<ZActiveSet> set = std::make_shared<ZActiveSet>();
    set->add(vertices[i]);
    activeset[vertices[i]] = set;
  }

  uint total = vertices.size();
  uint processed = 0;

  for(auto it = seeds.begin(); it != seeds.end(); ++it){
    activeset[it->first]->setLabel(it->second);
    processed ++;
  }

  while(!que.empty() && processed < total){
    const ZEdge& edge = que.top();
    que.pop();

    int from = edge.m_from;
    int to =  edge.m_to;

    shared_ptr<ZActiveSet> from_set = activeset[from];
    shared_ptr<ZActiveSet> to_set = activeset[to];

    if(from_set == to_set){
      continue;
    }

    if(to_set->getLabel() == 0){
      from_set->merge(*to_set.get());
      for(int v: to_set->data()){
        activeset[v] = from_set;
      }
      if(from_set->getLabel() != 0){
        processed += to_set->size();
      }
    } else if(from_set->getLabel() == 0){// to_set->getLabel() != 0
      to_set->merge(*from_set.get());
      for(int v: from_set->data()){
        activeset[v] = to_set;
      }
      processed += from_set->size();
    }
  }

  rv.resize(vertices.size());
  for(uint i = 0; i < vertices.size(); ++i){
    rv[i] = activeset[vertices[i]]->getLabel();
  }
}

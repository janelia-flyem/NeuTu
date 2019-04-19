#include<memory>
#include<unordered_map>
#include "zwatershedmst.h"

using std::shared_ptr;


void ZWatershedMST::run(vector<int>& rv, int vertices, const vector<ZEdge>& edges, const map<int,int>& seeds){
  std::priority_queue<ZEdge> que(edges.begin(), edges.end());

  vector<shared_ptr<ZActiveSet>> activeset(vertices + 1);
  for(int i = 1; i <= vertices; ++i){
    shared_ptr<ZActiveSet> set = std::make_shared<ZActiveSet>();
    set->add(i);
    activeset[i] = set;
  }

  int processed = 0;
  for(auto it = seeds.begin(); it != seeds.end(); ++it){
    activeset[it->first]->setLabel(it->second);
    processed ++;
  }

  while(!que.empty() && processed < vertices){
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

  rv.resize(vertices);
  for(uint i = 1; i <= vertices; ++i){
    rv[i-1] = activeset[i]->getLabel();
  }
}

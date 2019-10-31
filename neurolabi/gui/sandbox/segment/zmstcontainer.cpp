#include<iostream>
#include<fstream>
#include<ctime>
#include<set>
#include "zmstcontainer.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "flyem/zstackwatershedcontainer.h"


void ZMSTContainer::run(vector<int> &result_mapping, const ZStack &stack, const vector<ZStackObject *> &_seeds, shared_ptr<ZStack> label){
  shared_ptr<ZStack> label_stack = nullptr;
  int vertices = 0;

  if(label){
    label_stack = label;
    const float * pStack = label->array32();
    const float* const pEnd = pStack + label->getVoxelNumber();
    std::set<int> labels;
    for(;pStack != pEnd; ++pStack){
      if(*pStack){
        labels.insert(*pStack);
      }
    }
    vertices = labels.size();
  } else {
    label_stack = shared_ptr<ZStack>(new ZStack(FLOAT32, stack.getBoundBox(), 1));
    const uint8_t * pStack = stack.array8();
    const uint8_t* const pEnd = pStack + stack.getVoxelNumber();
    float* p = label_stack->array32();
    int index = 1;
    for(; pStack != pEnd; ++pStack, ++p){
      if(*pStack){
        *p = static_cast<float>(index++);
      }
    }
    vertices = index - 1;
  }

  int width = label_stack->width();
  int height = label_stack->height();
  int depth = label_stack->depth();
  int area =  width * height;

  const float* p = label_stack->array32();
  map<int,map<int,vector<double>>> cons;
  int nb[3] = {1, width, area};
  const uint8_t* pData = stack.array8();

  for(int d = 0; d < depth - 1; ++d){
    for(int h = 0; h < height - 1; ++h){
      for(int w = 0; w < width - 1; ++w){
        int offset = w + h * width + d * area;
        int u = static_cast<int>(p[offset]);
        if(u){
          for(int i = 0; i < 3; ++i){
            int v = static_cast<int>(p[offset + nb[i]]);
            if(v && u != v){
              double a = static_cast<double>(pData[offset]);
              double b = static_cast<double>(pData[offset + nb[i]]);
              cons[u][v].push_back(_weight(a,b));
            }
          }
        }
      }
    }
  }

  vector<ZEdge> edges;
  for(auto it = cons.begin(); it != cons.end(); ++it){
    int u = it->first;
    for(auto ip = it->second.begin(); ip != it->second.end(); ++ip){
      int v = ip->first;
      const vector<double>& weights = ip->second;
      double weight = std::accumulate(weights.begin(),weights.end(),0.0)/(weights.size() + 1e-6);
      edges.push_back(ZEdge(u,v,weight));
    }
  }

  std::map<int,int> seeds;
  p = label_stack->array32();

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();

  for (ZStackObject* obj: _seeds){
    ZStroke2d* seed = dynamic_cast<ZStroke2d*>(obj);
    if(seed){
      int label = seed->getLabel();
      const vector<int>& voxels = seed->toObject3d()->voxelArray();
      for(auto it = voxels.begin(); it !=  voxels.end();){
        int x = *it++ - ofx;
        int y = *it++ - ofy;
        int z = *it++ - ofz;
        int index = static_cast<int>(p[x + y * width + z * area]);
        if(index){
          seeds[index] = label;
        }
      }
    }
  }

  result_mapping.clear();

  clock_t start = clock();
  ZWatershedMST().run(result_mapping,vertices,edges,seeds);
  clock_t end = clock();
  //std::cout<<"Stack Size:"<<volume/1024.0/1024.0<<std::endl;
  std::cout<<"Graph Vertices: "<<vertices<<std::endl;
  std::cout<<"MST Running Time: "<<(end-start)*1000/CLOCKS_PER_SEC<<std::endl;
}


shared_ptr<ZStack> ZMSTContainer::run(const ZStack& stack, const vector<ZStackObject *> &_seeds, shared_ptr<ZStack> label){
  shared_ptr<ZStack> label_stack = nullptr;
  int vertices = 0;

  if(label){
    label_stack = label;
    const float * pStack = label->array32();
    const float* const pEnd = pStack + label->getVoxelNumber();
    std::set<int> labels;
    for(;pStack != pEnd; ++pStack){
      if(*pStack){
        labels.insert(*pStack);
      }
    }
    vertices = labels.size();
  } else {
    label_stack = shared_ptr<ZStack>(new ZStack(FLOAT32, stack.getBoundBox(), 1));
    const uint8_t * pStack = stack.array8();
    const uint8_t* const pEnd = pStack + stack.getVoxelNumber();
    float* p = label_stack->array32();
    int index = 1;
    for(; pStack != pEnd; ++pStack, ++p){
      if(*pStack){
        *p = static_cast<float>(index++);
      }
    }
    vertices = index - 1;
  }

  int width = label_stack->width();
  int height = label_stack->height();
  int depth = label_stack->depth();
  int area =  width * height;

  const float* p = label_stack->array32();
  map<int,map<int,vector<double>>> cons;
  int nb[3] = {1, width, area};
  const uint8_t* pData = stack.array8();

  for(int d = 0; d < depth - 1; ++d){
    for(int h = 0; h < height - 1; ++h){
      for(int w = 0; w < width - 1; ++w){
        int offset = w + h * width + d * area;
        int u = static_cast<int>(p[offset]);
        if(u){
          for(int i = 0; i < 3; ++i){
            int v = static_cast<int>(p[offset + nb[i]]);
            if(v && u != v){
              double a = static_cast<double>(pData[offset]);
              double b = static_cast<double>(pData[offset + nb[i]]);
              cons[u][v].push_back(_weight(a,b));
            }
          }
        }
      }
    }
  }

  vector<ZEdge> edges;
  for(auto it = cons.begin(); it != cons.end(); ++it){
    int u = it->first;
    for(auto ip = it->second.begin(); ip != it->second.end(); ++ip){
      int v = ip->first;
      const vector<double>& weights = ip->second;
      double weight = std::accumulate(weights.begin(),weights.end(),0.0)/(weights.size() + 1e-6);
      edges.push_back(ZEdge(u,v,weight));
    }
  }

  std::map<int,int> seeds;
  p = label_stack->array32();

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();

  for (ZStackObject* obj: _seeds){
    ZStroke2d* seed = dynamic_cast<ZStroke2d*>(obj);
    if(seed){
      int label = seed->getLabel();
      const vector<int>& voxels = seed->toObject3d()->voxelArray();
      for(auto it = voxels.begin(); it !=  voxels.end();){
        int x = *it++ - ofx;
        int y = *it++ - ofy;
        int z = *it++ - ofz;
        int index = static_cast<int>(p[x + y * width + z * area]);
        if(index){
          seeds[index] = label;
        }
      }
    }
  }

  std::vector<int> segmentation;

  clock_t start = clock();
  ZWatershedMST().run(segmentation,vertices,edges,seeds);
  clock_t end = clock();
  //std::cout<<"Stack Size:"<<volume/1024.0/1024.0<<std::endl;
  std::cout<<"Graph Vertices: "<<vertices<<std::endl;
  std::cout<<"MST Running Time: "<<(end-start)*1000/CLOCKS_PER_SEC<<std::endl;

  shared_ptr<ZStack> rv = shared_ptr<ZStack>(stack.clone());
  rv->setZero();
  uint8_t* q = rv->array8();

  p = label_stack->array32();
  const float* const pEnd = p + label_stack->getVoxelNumber();

  for(; p != pEnd; ++p, ++q){
    int old_label = static_cast<int>(*p);
    if(old_label){
      int new_label = segmentation[old_label - 1];
      *q = new_label;
    }
  }

  return rv;
}


double ZMSTContainer::_weight(double a, double b) const{
  //double connection = std::exp(-0.05*(std::abs(a-b))); //a==b-> 1, |a-b|==10 -> 0.6 |a-b|==20->0.37
  //double intensity = std::exp(-2*(1-(a+b)/500));//255->1, 200->0.64, 100->0.3
  //return 0.1 * connection + intensity;
  return a + b;
}

/*
void ZMSTContainer::consume(const ZStack *seg, const ZStack *img){

  if(seg.kind() == GREY){
    _consume<uint8>(seg.array8(),seg,img);
  } else if(seg.kind() == GREY16){
    _consume<uint16>(seg.array16(),seg,img);
  } else if(seg.kind() == FLOAT32){
    _consume<float>(seg.array32(),seg,img);
  }
}


template<typename T>
void ZSuperVoxelManager::_consume(const T* array, const ZStack& seg, const ZStack& img){
  int width = seg.width();
  int height = seg.height();
  int depth = seg.depth();
  int slice = width * height;

  ZIntPoint offset = seg.getOffset();
  int ofx = offset.getX();
  int ofy = offset.getY();
  int ofz = offset.getZ();

  std::cout<<"Creating Super Voxel Encoders..."<<std::endl;

  for(int d = 0; d < depth; ++d){//construct encoders
    for(int h = 0; h < height; ++h){
      int index = d * slice + h * width;
      int prev = 0, x1 = 0, x2 = 0;
      for(int w = 0; w < width; ++w){
        int v = array[index++];
        if(v == prev){
          ++x2;
        } else {
          if(prev){
            _addSegment(prev,ofz+d,ofy+h,ofx+x1,ofx+x2,offset);
          }
          x1 = x2 = w;
          prev = v;
        }
      }
      if(prev){
        _addSegment(prev,ofz+d,ofy+h,ofx+x1,ofx+x2,offset);
      }
    }
  }

  std::cout<<"Creating Super Voxel Encoders Done"<<std::endl;

  //make vertices
  for(auto it =  m_data.begin(); it != m_data.end(); ++it){
    m_vertices.push_back(it->first);
  }

  //find connections
  map<int,map<int,vector<double>>> cons;

  const float* pSeg = seg.array32();
  const uint8_t* pImg = img.array8();

  int nb[3] = {1, width, slice};

  std::cout<<"Computing Adj Matrix..."<<std::endl;

  for(int d = 0; d < depth -1; ++d){
    for(int h = 0; h < height -1; ++h){
      for(int w = 0; w < width -1; ++w){
        int index = w + h * width + d * slice;
        int v = pSeg[index];
        if(v){
          for(int i = 0; i < 3; ++i){
            int u = pSeg[index + nb[i]];
            if(u && u != v){
              int a = std::min(u,v);
              int b = std::max(u,v);
              double va = static_cast<double>(pImg[index]);
              double vb = static_cast<double>(pImg[index + nb[i]]);
              double weight = _weight(va,vb);
              cons[a][b].push_back(weight);
            }
          }
        }
      }
    }
  }

  //make edges
  for(auto it = cons.begin(); it != cons.end(); ++it){
    for(auto ip = it->second.begin(); ip != it->second.end(); ++ip){
      double weight = std::accumulate(ip->second.begin(),ip->second.end(),0.0)/(ip->second.size()+0.001);
      //double weight = std::accumulate(ip->second.begin(), ip->second.end(), 0.0);
      m_edges.push_back(ZEdge(it->first, ip->first, weight));
    }
  }

  std::cout<<"Computing Adj Matrix Done"<<std::endl;
}


void ZSuperVoxelManager::_addSegment(int v, int z, int y, int x1, int x2, const ZIntPoint& offset){
  auto it = m_data.find(v);
  if(it != m_data.end()){
    it->second->addSegment(z,y,x1,x2);
  } else {
    auto encoder = shared_ptr<ZSegmentationEncoder>(new ZSegmentationEncoder(offset));
    encoder->addSegment(z,y,x1,x2);
    m_data[v] = encoder;
  }
}





int ZSuperVoxelManager::contains(int x, int y, int z) const{
  for(auto it = m_data.begin(); it != m_data.end(); ++it){
    int label = it->first;
    shared_ptr<ZSegmentationEncoder> encoder = it->second;
    if(encoder->contains(x,y,z)){
      return label;
    }
  }
  return 0;
}


const shared_ptr<ZSegmentationEncoder>& ZSuperVoxelManager::getSuperVoxel(int label) const{
  auto it = m_data.find(label);
  if(it != m_data.end()){
    return it->second;
  }
  static shared_ptr<ZSegmentationEncoder> empty = nullptr;
  return empty;
}*/

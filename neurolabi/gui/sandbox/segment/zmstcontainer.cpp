#include<iostream>
#include "zmstcontainer.h"
#include "zstroke2d.h"
#include "zobject3d.h"
#include "flyem/zstackwatershedcontainer.h"


shared_ptr<ZStack> ZMSTContainer::run(const ZStack& stack, const vector<ZStackObject *> &_seeds){
  size_t volume = stack.getVoxelNumber();
  bool super_voxel = (volume >= 100000);
  std::cout<<"Super Voxel is Enable: "<<super_voxel<<std::endl;

  shared_ptr<ZStack> label_stack = nullptr;
  if(super_voxel){
    uint start_label = 1;
    int stride_x = 2;
    int stride_y = 2;
    int stride_z = 2;
    if(stack.width() < 100){
      stride_x = 1;
    }
    if(stack.height() < 100){
      stride_y = 1;
    }
    if(stack.height() < 100){
      stride_z =1;
    }
    label_stack = _createSuperVoxel(stack, start_label, stride_x,stride_y,stride_z);
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
  }

  std::set<int> _vertices;
  const float* p = label_stack->array32();
  const float* const pEnd = p + label_stack->getVoxelNumber();
  for(; p != pEnd; ++p){
    if(*p){
      _vertices.insert(static_cast<int>(*p));
    }
  }
  vector<int> vertices(_vertices.begin(),_vertices.end());

  std::cout<<"Graph Vertices: "<<vertices.size()<<std::endl;

  int width = label_stack->width();
  int height = label_stack->height();
  int depth = label_stack->depth();
  int area =  width * height;

  p = label_stack->array32();
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

  ZWatershedMST().run(segmentation,vertices,edges,seeds);

  p = label_stack->array32();
  map<int,int> old_to_new;
  for(uint i = 0; i < vertices.size(); ++i){
    old_to_new[vertices[i]] = segmentation[i];
  }

  shared_ptr<ZStack> rv = shared_ptr<ZStack>(stack.clone());
  rv->setZero();
  uint8_t* q = rv->array8();
  p = label_stack->array32();
  for(; p != pEnd; ++p, ++q){
    int old_label = static_cast<int>(*p);
    if(old_label){
      int new_label = old_to_new[old_label];
      *q = new_label;
    }
  }

  return rv;
}


shared_ptr<ZStack> ZMSTContainer::_createSuperVoxel(const ZStack &stack, uint& start_label,int sx, int sy, int sz) const {
  shared_ptr<ZStack> rv = shared_ptr<ZStack>(new ZStack(FLOAT32, stack.getBoundBox(), 1));
  std::pair<bool,shared_ptr<ZStack>> status_seeds = _seedsFromLocalMaximum(stack,sx,sy,sz);

  if(status_seeds.first){
     ZStackWatershedContainer container(const_cast<ZStack*>(&stack));
     container.setDsMethod("Min(ignore zero)");
     container.addSeed(*(status_seeds.second));
     container.run();
     ZStackPtr presult = container.getResultStack();
     uint8_t* src = presult->array8();
     float* dst = rv->array32();
     std::set<int> labels;
     for(uint i = 0; i < presult->getVoxelNumber(); ++i){
      if(src[i]){
        dst[i] = src[i] + start_label - 1;
        labels.insert(src[i]);
      }
     }
     start_label += labels.size();
  } else {
    ZIntCuboid box = stack.getBoundBox();
    ZIntCuboid box_a = box;
    ZIntCuboid box_b = box;
    int width = box.getWidth();
    int height = box.getHeight();
    int depth = box.getDepth();
    if(height >= width && height >= depth){
      box_a.setLastY(box.getFirstCorner().getY() + height/2);
      box_b.setFirstY(box.getFirstCorner().getY() + height/2 + 1);
    } else if(width >= height && width >= depth){
      box_a.setLastX(box.getFirstCorner().getX() + width/2);
      box_b.setFirstX(box.getFirstCorner().getX() + width/2 + 1);
    } else {
      box_a.setLastZ(box.getFirstCorner().getZ() + depth/2);
      box_b.setFirstZ(box.getFirstCorner().getZ() + depth /2 + 1);
    }
    shared_ptr<ZStack> stack_a = shared_ptr<ZStack>(stack.makeCrop(box_a));
    shared_ptr<ZStack> stack_b = shared_ptr<ZStack>(stack.makeCrop(box_b));

    shared_ptr<ZStack> rv_a = _createSuperVoxel(*stack_a,start_label,sx,sy,sz);
    shared_ptr<ZStack> rv_b = _createSuperVoxel(*stack_b,start_label,sx,sy,sz);

    int x0 = rv_a->getOffset().getX();
    int y0 = rv_a->getOffset().getY();
    int z0 = rv_a->getOffset().getZ();
    rv->setBlockValue(x0,y0,z0,rv_a.get());
    int x1 = rv_b->getOffset().getX();
    int y1 = rv_b->getOffset().getY();
    int z1 = rv_b->getOffset().getZ();
    rv->setBlockValue(x1,y1,z1,rv_b.get());
  }
  return rv;
}

std::pair<bool, shared_ptr<ZStack>> ZMSTContainer::_seedsFromLocalMaximum(const ZStack &stack,int sx, int sy, int sz) const{
  shared_ptr<ZStack> rv = shared_ptr<ZStack>(stack.clone());
  rv->setZero();

  int width = stack.width();
  int height = stack.height();
  int depth  = stack.depth();
  int area = width * height;

  const uint8_t* p = stack.array8();
  uint8_t* q = rv->array8();
  uint k = 1;

  int stride_x = sx;
  int stride_y = sy;
  int stride_z = sz;

  for(int d = 1; d < depth - 1; d += stride_z){
    for(int h = 1; h < height - 1; h += stride_y){
      size_t index = 1 + h * width + d * area;
      for(int w = 1; w < width - 1; w += stride_x, index += stride_x){
        bool ok = true;
        uint8 v = p[index];
        if(v){
          for(int k = -1; k <= 1; ++k){
            for(int j = -1; j <= 1; ++j){
              for(int i = -1; i <= 1; ++i){
                if( i==0 && j==0 && k==0){
                  continue;
                }
                if(p[index + k*area + j*width +i] >= v){
                  ok = false;
                  break;
                }
              }
            }
          }
          if(ok){
            if( k > 255){
              return std::make_pair(false,nullptr);
            }
            q[index] = k++;
            //w += 1;
            //index += 1;
          }
        }
      }
    }
  }
  return std::make_pair(true,rv);
}


double ZMSTContainer::_weight(double a, double b) const{
  double connection = std::exp(-0.05*(std::abs(a-b))); //a==b-> 1, |a-b|==10 -> 0.6 |a-b|==20->0.37
  double intensity = std::exp(-2*(1-(a+b)/500));//255->1, 200->0.64, 100->0.3
  return 0.1 * connection + intensity;
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

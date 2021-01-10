#include "zsegmentationencoder.h"


ZSegmentationEncoder::ZSegmentationEncoder(const ZIntPoint &offset):
m_offset(offset), m_minx(INT32_MAX), m_maxx(0),
m_miny(INT32_MAX), m_maxy(0), m_minz(INT32_MAX), m_maxz(0){

}


void ZSegmentationEncoder::consume(const ZStack &stack){
  const uint8_t* p = stack.array8();
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();
  int slice = width*height;
  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();
  int index = 0;
  int v = 0;

  for(int d = 0; d < depth; ++d){
    for(int h = 0; h < height; ++h){
      index = d * slice + h * width;
      bool flag = false;
      int x1 = 0;
      int x2 = 0;
      for(int w = 0; w < width; ++w){
        v = p[index++];
        if(v){
          if(!flag){
            x1 = w;
            x2 = x1;
            flag = true;
          } else {
            ++x2;
          }
        } else {
          if(flag){
            flag = false;
            addSegment(d+ofz,h+ofy,x1+ofx,x2+ofx);
          }
        }
      }
      if(flag){
        addSegment(d+ofz,h+ofy,x1+ofx,x2+ofx);
      }
    }
  }
}


void ZSegmentationEncoder::addSegment(int z, int y, int start, int end){
  if(start > end){
    return;
  }
  _maybe_update_bound_box(z,y,start,end);

  vector<int>& segments = getSegment(z,y);//m_data[z][y];

  if(segments.size() ==0 ){
    segments.push_back(start);
    segments.push_back(end);
    return;
  }

  uint i = 0;
  while(i < segments.size() - 1 && segments[i] <= start){
    i += 2;
  }

  if(i < segments.size() - 1){// segments[i] > start
    uint j = i;
    while(j < segments.size() - 1 && segments[j] <= end + 1){
      j += 2;
    }
    if(j > 0){//segments[j]> end + 1
      if(start > segments[j-1] + 1){
        segments.insert(segments.begin() + j, start);
        segments.insert(segments.begin() + j + 1, end);
        return;
      } else {
         segments[j-1] = std::max(segments[j-1], end);
        if(j > i + 1)
          segments.erase(segments.begin() + i + 1, segments.begin() + j - 1);
      }
    } else {
      segments.insert(segments.begin(), start);
      segments.insert(segments.begin() + 1, end);
      return;
    }
    if(i > 0 && start <= segments[i-1] + 1 && j > i + 1){
        segments.erase(segments.begin() + i - 1, segments.begin() + j - 1);
        return;
    }
    if(i > 0 && start > segments[i-1] + 1){
        segments[i] = start;
        return;
    }
    if(i == 0){
        segments[0] = start;
        return;
    }
  } else {
    if(start > segments.back() + 1){
      segments.push_back(start);
      segments.push_back(end);
      return;
    }
    segments.back() = std::max(segments.back(), end);
    return;
  }
}


void ZSegmentationEncoder::_maybe_update_bound_box(int z, int y, int start, int end){
  m_maxz = m_maxz > z ? m_maxz : z;
  m_minz = m_minz < z ? m_minz : z;
  m_maxy = m_maxy > y ? m_maxy : y;
  m_miny = m_miny < y ? m_miny : y;
  m_maxx = m_maxx > end ? m_maxx : end;
  m_minx = m_minx < start ? m_minx : start;
}


vector<int>& ZSegmentationEncoder::getSegment(int z, int y){
  z -= m_offset.getZ();
  y -= m_offset.getY();

  assert(z >= 0 && y >= 0);

  if(z >= static_cast<int>(m_data.size())){
    m_data.resize(z + 1);
  }
  if(y >= static_cast<int>(m_data[z].size())){
    m_data[z].resize(y + 1);
  }
  return m_data[z][y];
}


const vector<int>& ZSegmentationEncoder::getSegment(int z, int y)const{
  z -= m_offset.getZ();
  y -= m_offset.getY();
  static vector<int> empty;
  if(z >= 0 && z < static_cast<int>(m_data.size())){
    if(y >= 0 && y < static_cast<int> (m_data[z].size())){
      return m_data[z][y];
    }
  }
  return empty;
}


bool ZSegmentationEncoder::contains(int x, int y, int z) const{
  const vector<int>& segments = getSegment(z,y);
  for(int i = 0; i < static_cast<int>(segments.size()) - 1; i += 2){
    if(x >= segments[i] && x <= segments[i+1]){
      return true;
    }
  }
  return false;
  /*int L = 0;
  int R = segments.size() - 2;
  while( L<= R){
    int M = L + 2 * ((R-L) / 4);
    if(x >= segments[M]){
      if(x <= segments[M+1]){
        return true;
      } else {
        L = M + 2;
      }
    } else {
      R = M-2;
    }
  }
  return false;*/
}


void ZSegmentationEncoder::unify(const ZSegmentationEncoder &encoder){
  const ZIntCuboid& boxa = encoder.getBoundBox();
  const ZIntCuboid& boxb = getBoundBox();

  ZIntCuboid boxt = boxa;
  boxt.join(boxb);
  ZSegmentationEncoder tmp(boxt.getMinCorner());

  int z0 = boxa.getMinCorner().getZ();
  int z1 = boxa.getMaxCorner().getZ();
  int y0 = boxa.getMinCorner().getY();
  int y1 = boxa.getMaxCorner().getY();

  for(int z = z0; z <= z1; ++z){
    for(int y = y0; y <= y1; ++y){
      const vector<int>& segments = encoder.getSegment(z,y);
      for(uint i = 0; i < segments.size(); i += 2){
        tmp.addSegment(z,y,segments[i],segments[i+1]);
      }
    }
  }

  z0 = boxb.getMinCorner().getZ();
  z1 = boxb.getMaxCorner().getZ();
  y0 = boxb.getMinCorner().getY();
  y1 = boxb.getMaxCorner().getY();

  for(int z = z0; z <= z1; ++z){
    for(int y = y0; y <= y1; ++y){
      const vector<int>& segments = getSegment(z,y);
      for(uint i = 0; i < segments.size(); i += 2){
        tmp.addSegment(z,y,segments[i],segments[i+1]);
      }
    }
  }

  *this = tmp;
}


void ZSegmentationEncoder::labelStack(ZStack &stack, int value) const{
  if(stack.kind() == GREY){
    labelStack<uint8>(stack,stack.array8(),value);
  } else if (stack.kind() == GREY16){
    labelStack<uint16>(stack,stack.array16(),value);
  } else if(stack.kind() == FLOAT32){
    labelStack<float>(stack,stack.array32(),value);
  }
}


template<typename T>
void ZSegmentationEncoder::labelStack(ZStack &stack, T* array, int value) const{
  int width = stack.width();
  int height = stack.height();
  int area = width * height;
  T* pdata = array;
  int index = 0;

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();

  int min_y = getBoundBox().getMinCorner().getY();
  int max_y = getBoundBox().getMaxCorner().getY();
  int min_z = getBoundBox().getMinCorner().getZ();
  int max_z = getBoundBox().getMaxCorner().getZ();

  for(int z = min_z; z <= max_z; ++z){
    for(int y = min_y; y <= max_y; ++y){
      const vector<int>& segments = getSegment(z,y);
      if(segments.size() < 2)
        continue;
      for(uint i = 0; i < segments.size() - 1; i += 2){
        int j = segments[i] - ofx;
        index = (z-ofz) * area + (y-ofy) * width + j;
        for(; j <= segments[i+1] - ofx; ++j, ++index){
          pdata[index]  = value;
        }
      }
    }
  }
}


ZSegmentationEncoder* ZSegmentationEncoder::clone()const{
  ZSegmentationEncoder* rv = new ZSegmentationEncoder(m_offset);
  rv->m_data = m_data;
  rv->m_maxx = m_maxx;
  rv->m_maxy = m_maxy;
  rv->m_maxz = m_maxz;
  rv->m_minx = m_minx;
  rv->m_miny = m_miny;
  rv->m_minz = m_minz;
  return rv;
}


double ZSegmentationEncoder::memUsage()const{
  double rv = m_data.size() * sizeof(vector<vector<int>>);
  for(auto it = m_data.begin(); it != m_data.end(); ++it){
    rv += it->size() * sizeof(vector<int>);
    for(auto ip = it->begin(); ip != it->end(); ++ip){
      rv += ip->size() * sizeof(int);
    }
  }
  return rv;
}


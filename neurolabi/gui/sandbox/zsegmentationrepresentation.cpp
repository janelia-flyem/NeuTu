#include "zsegmentationrepresentation.h"


ZSegmentationEncoder::ZSegmentationEncoder(){

}


ZSegmentationEncoderRLX::ZSegmentationEncoderRLX(){

}


void ZSegmentationEncoder::add(const vector<int> &xs, const vector<int> &ys, const vector<int> &zs){
    vector<int>::const_iterator itx = xs.begin(), ity = ys.begin(), itz = zs.begin();
    while(itx != xs.end() && ity != ys.end() && itz != zs.end())add(*itx++, *ity++, *itz++);
}


void ZSegmentationEncoder::remove(const vector<int> &xs, const vector<int> &ys, const vector<int> &zs){
    vector<int>::const_iterator itx = xs.begin(), ity = ys.begin(), itz = zs.begin();
    while(itx != xs.end() && ity != ys.end() && itz != zs.end())remove(*itx++, *ity++, *itz++);
}


ZSegmentationEncoder* ZSegmentationEncoderRLX::clone()const{
  ZSegmentationEncoderRLX* rv = new ZSegmentationEncoderRLX();
  rv->m_data = m_data;
  return rv;
}


void ZSegmentationEncoderRLX::clear(){
  m_data.clear();
}


double ZSegmentationEncoderRLX::memUsage()const{
  double rv = 0;
  for(data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    rv += 8;
    for(slice_type::const_iterator ip = it->second.begin(); ip != it->second.end(); ++ip){
      rv += 8;
      rv += ip->second.size() * 4;
    }
  }
  return rv;
}


void ZSegmentationEncoderRLX::add(const ZIntPoint &point){
  _addSegment(point.getZ(),point.getY(),point.getX(),point.getX());
}


bool ZSegmentationEncoderRLX::contains(const ZIntPoint &point) const{
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  data_type::const_iterator ita = m_data.find(z);
  if(ita == m_data.end()){
    return false;
  }

  slice_type::const_iterator itb = ita->second.find(y);
  if(itb == ita->second.end()){
    return false;
  }

  const Encoder_type& segments = itb->second;

  int L = 0;
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
  return false;
}


void ZSegmentationEncoderRLX::labelStack(ZStack &stack, int value) const{
  int width = stack.width();
  int height = stack.height();
  //int depth = stack.depth();
  int area = width*height;
  uint8_t* pdata = stack.array8();
  int index = 0;
  int z = 0;
  int y = 0;
  //int x = 0;

  for(data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    const slice_type& slice = it->second;
    z = it->first;
    for(slice_type::const_iterator ip = slice.begin(); ip != slice.end(); ++ip){
      const Encoder_type& segments = ip->second;
      y = ip->first;
      for(uint i = 0; i < segments.size() - 1; i += 2){
        int j = segments[i];
        index = z * area + y * width + j;
        for(; j <= segments[i+1]; ++j, ++index){
          pdata[index]  = value;
        }
      }
    }
  }
}


void ZSegmentationEncoderRLX::remove(const ZIntPoint &point){
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  auto ita = m_data.find(z);
  if(ita == m_data.end()){
    return;
  }

  auto itb = ita->second.find(y);
  if(itb == ita->second.end()){
    return;
  }

  Encoder_type& segments = itb->second;

  int L = 0;
  int R = segments.size() - 2;
  while(L <= R){
    int M = L + 2 * ((R-L) / 4);
    if(x >= segments[M]){
      if(x == segments[M]){
        if(segments[M] == segments[M+1]){
          segments.erase(segments.begin()+M);
          segments.erase(segments.begin()+M);
        } else {
          segments[M] += 1;
        }
      } else if(x == segments[M+1]){
         segments[M+1] -= 1;
      } else if ( x < segments[M+1]){
        segments.insert(segments.begin() + M + 1, x-1);
        segments.insert(segments.begin() + M + 2, x+1);
      } else {
        L = M + 2;
      }
    } else {
      R = M-2;
    }
  }
}


void ZSegmentationEncoderRLX::unify(const ZSegmentationEncoder &segmentation_encoder){
  if(segmentation_encoder.type() == type()){
    const ZSegmentationEncoderRLX* p = (const ZSegmentationEncoderRLX*)&segmentation_encoder;
    for(data_type::const_iterator it = p->m_data.begin(); it != p->m_data.end(); ++it){
      const slice_type& slice = it->second;
      int z = it->first;
      for(slice_type::const_iterator ip = slice.begin(); ip != slice.end(); ++ip){
        const Encoder_type& segments = ip->second;
        int y = ip->first;
        for(uint i = 0; i < segments.size() - 1; i += 2){
          _addSegment(z,y,segments[i],segments[i+1]);
        }
      }
    }
  }
}


void ZSegmentationEncoderRLX::_addSegment(int z, int y, int start, int end){

  if(start > end){
    return;
  }
  Encoder_type& segments = m_data[z][y];

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

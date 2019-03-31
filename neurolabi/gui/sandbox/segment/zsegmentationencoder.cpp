#include "zsegmentationencoder.h"


ZSegmentationEncoder::ZSegmentationEncoder(){

}


void ZSegmentationEncoder::add(const vector<int> &xs, const vector<int> &ys, const vector<int> &zs){
    vector<int>::const_iterator itx = xs.begin(), ity = ys.begin(), itz = zs.begin();
    while(itx != xs.end() && ity != ys.end() && itz != zs.end())add(*itx++, *ity++, *itz++);
}


void ZSegmentationEncoder::initBoundBox(const ZIntCuboid &box){
  m_init_box = box;
}


ZSegmentationEncoderRLX::ZSegmentationEncoderRLX(){
  m_minx = INT32_MAX;
  m_maxx = 0;
  m_miny = INT32_MAX;
  m_maxy = 0;
  m_minz = INT32_MAX;
  m_maxz = 0;
}


/*
ZSegmentationEncoder* ZSegmentationEncoderRLX::clone()const{
  ZSegmentationEncoderRLX* rv = new ZSegmentationEncoderRLX();
  rv->m_data = m_data;
  return rv;
}
*/



void ZSegmentationEncoderRLX::add(const ZIntPoint &point){
  _addSegment(point.getZ(),point.getY(),point.getX(),point.getX());
}


bool ZSegmentationEncoderRLX::contains(const ZIntPoint &point) const{
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();
  /*if(!hasSegment(z,y)){
    return false;
  }*/
  const vector<int>& segments = getSegment(z,y);
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

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();


  int min_y = getBoundBox().getFirstCorner().getY();
  int max_y = getBoundBox().getLastCorner().getY();
  int min_z = getBoundBox().getFirstCorner().getZ();
  int max_z = getBoundBox().getLastCorner().getZ();

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


void ZSegmentationEncoderRLX::unify(const ZSegmentationEncoder &segmentation_encoder){
  if(segmentation_encoder.type() == type()){
    const ZSegmentationEncoderRLX* p = (const ZSegmentationEncoderRLX*)&segmentation_encoder;
    const ZIntCuboid& box = p->getBoundBox();
    int z0 = box.getFirstCorner().getZ();
    int z1 = box.getLastCorner().getZ();
    int y0 = box.getFirstCorner().getY();
    int y1 = box.getLastCorner().getY();

    for(int z = z0; z <= z1; ++z){
      for(int y = y0; y <= y1; ++y){
        const vector<int>& segments = p->getSegment(z,y);
        for(uint i = 0; i < segments.size(); i += 2){
          _addSegment(z,y,segments[i],segments[i+1]);
        }
      }
    }
  }
}


void ZSegmentationEncoderRLX::_maybe_update_bound_box(int z, int y, int start, int end){
  m_maxz = m_maxz > z ? m_maxz : z;
  m_minz = m_minz < z ? m_minz : z;
  m_maxy = m_maxy > y ? m_maxy : y;
  m_miny = m_miny < y ? m_miny : y;
  m_maxx = m_maxx > end ? m_maxx : end;
  m_minx = m_minx < start ? m_minx : start;
}


void ZSegmentationEncoderRLX::consume(const ZStack &stack){
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
            _addSegment(d+ofz,h+ofy,x1+ofx,x2+ofx);
          }
        }
      }
      if(flag){
        _addSegment(d+ofz,h+ofy,x1+ofx,x2+ofx);
      }
    }
  }
}


void ZSegmentationEncoderRLX::_addSegment(int z, int y, int start, int end){

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


double ZSegmentationEncoderRLXTwoStageHashing::memUsage()const{
  double rv = 0;
  for(uint i = 0; i < m_data.bucket_count(); ++i){
    rv += m_data.bucket_size(i) * sizeof(unordered_map<int,vector<int>>);
  }
  for(auto it = m_data.begin(); it != m_data.end(); ++it){
    for(uint i= 0 ; i < it->second.bucket_count(); ++i){
      rv += it->second.bucket_size(i) * sizeof(vector<int>);
    }
    for(auto ip = it->second.begin(); ip != it->second.end(); ++ip){
      rv += ip->second.size() * sizeof(int);
    }
  }
  return rv;
}


ZSegmentationEncoder* ZSegmentationEncoderRLXTwoStageHashing::clone()const{
  ZSegmentationEncoderRLXTwoStageHashing* rv = new ZSegmentationEncoderRLXTwoStageHashing();
  rv->m_data = m_data;
  return rv;
}


vector<int>& ZSegmentationEncoderRLXTwoStageHashing::getSegment(int z, int y){
  return m_data[z][y];
}


const vector<int>& ZSegmentationEncoderRLXTwoStageHashing::getSegment(int z, int y)const{
  static vector<int> empty;
  auto it = m_data.find(z);
  if(it != m_data.end()){
    const unordered_map<int,vector<int>>& slice = it->second;
    auto ip = slice.find(y);
    if(ip != slice.end()){
      return ip->second;
    }
  }
  return empty;
}


bool ZSegmentationEncoderRLXTwoStageHashing::hasSegment(int z, int y) const{
  auto it = m_data.find(z);
  if(it == m_data.end()){
    return false;
  }
  const unordered_map<int,vector<int>>& slice = it->second;
  auto ip = slice.find(y);
  if(ip == slice.end()){
    return false;
  }
  return true;
}


ZSegmentationEncoder* ZSegmentationEncoderRLXOneStageHashing::clone()const{
  ZSegmentationEncoderRLXOneStageHashing* rv = new ZSegmentationEncoderRLXOneStageHashing();
  rv->m_data = m_data;
  return rv;
}


vector<int>& ZSegmentationEncoderRLXOneStageHashing::getSegment(int z, int y){
  return m_data[make_tuple(z,y)];
}


const vector<int>& ZSegmentationEncoderRLXOneStageHashing::getSegment(int z, int y)const{
  static vector<int> empty;
  auto it = m_data.find(make_tuple(z,y));
  if(it != m_data.end()){
    return it->second;
  }
  return empty;
}


bool ZSegmentationEncoderRLXOneStageHashing::hasSegment(int z, int y) const{
  return m_data.find(make_tuple(z,y)) != m_data.end();
}


double ZSegmentationEncoderRLXOneStageHashing::memUsage()const{
  double rv = 0;
  for(uint i = 0; i < m_data.bucket_count(); ++i){
    rv += m_data.bucket_size(i) * sizeof(vector<int>);
  }
  for(auto it = m_data.begin(); it != m_data.end(); ++it){
    rv += it->second.size() * sizeof(int);
  }
  return rv;
}


ZSegmentationEncoder* ZSegmentationEncoderRLXVector::clone()const{
  ZSegmentationEncoderRLXVector* rv = new ZSegmentationEncoderRLXVector();
  rv->m_data = m_data;
  return rv;
}


vector<int>& ZSegmentationEncoderRLXVector::getSegment(int z, int y){

  int z0 = m_init_box.getFirstCorner().getZ();
  int y0 = m_init_box.getFirstCorner().getY();

  z = z - z0;
  y = y - y0;
  if(z >= (int)m_data.size()){
    m_data.resize(z + 1);
  }
  if(y >= (int)m_data[z].size()){
    m_data[z].resize(y + 1);
  }
  //assert(z >= 0 && y >= 0);
  return m_data[z][y];
}


const vector<int>& ZSegmentationEncoderRLXVector::getSegment(int z, int y)const{
  int z0 = m_init_box.getFirstCorner().getZ();
  int y0 = m_init_box.getFirstCorner().getY();

  z = z - z0;
  y = y - y0;
  static vector<int> empty;
  if(z >=0 && z < (int)m_data.size()){
    if(y >=0 && y < (int) m_data[z].size()){
      return m_data[z][y];
    }
  }
  return empty;
}


bool ZSegmentationEncoderRLXVector::hasSegment(int z, int y) const{
  int z0 = m_init_box.getFirstCorner().getZ();
  int y0 = m_init_box.getFirstCorner().getY();
  z = z - z0;
  y = y - y0;
  if(z >= 0  && z < (int)m_data.size()){
    if(y >= 0 && y < (int)m_data[z].size()){
      return true;
    }
  }
  return false;
}


double ZSegmentationEncoderRLXVector::memUsage()const{
  //return m_data.
  double rv = m_data.size() * sizeof(vector<vector<int>>);
  for(auto it = m_data.begin(); it != m_data.end(); ++it){
    rv += it->size() * sizeof(vector<int>);
    for(auto ip = it->begin(); ip != it->end(); ++ip){
      rv += ip->size() * sizeof(int);
    }

  }
  return rv;
}


ZSegmentationEncoderBitMap::ZSegmentationEncoderBitMap(){
  m_minx = INT32_MAX;
  m_maxx = 0;
  m_miny = INT32_MAX;
  m_maxy = 0;
  m_minz = INT32_MAX;
  m_maxz = 0;
  m_inited = false;
}


void ZSegmentationEncoderBitMap::add(const ZIntPoint &point){
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  _maybe_update_bound_box(z,y,x,x);

  x -= m_init_box.getFirstCorner().getX();
  y -= m_init_box.getFirstCorner().getY();
  z -= m_init_box.getFirstCorner().getZ();

  int width = m_init_box.getWidth();
  int height = m_init_box.getHeight();
  int slice = width * height;

  int index = z * slice + y * width + x;
  int out_index = index / (8*sizeof(int));
  int in_index = index % (8*sizeof(int));

  if(!m_inited){
    m_data.resize(m_init_box.getVolume()/(8*sizeof(int)) + 1);
    m_inited = true;
    //std::cout<<m_data.size()<<std::endl;
  }
  /*if(out_index >= (int)m_data.size()){
    m_data.resize(out_index + 1);
  }*/
  m_data[out_index] |= (1 << in_index);
}


void ZSegmentationEncoderBitMap::consume(const ZStack &stack){
  initBoundBox(stack.getBoundBox());

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
      for(int w = 0; w < width; ++w){
        v = p[index++];
        if(v){
          add(ZIntPoint(w+ofx,h+ofy,d+ofz));
        }
      }
    }
  }
}


ZSegmentationEncoder* ZSegmentationEncoderBitMap::clone()const{
  ZSegmentationEncoderBitMap* rv = new ZSegmentationEncoderBitMap();
  rv->m_data = m_data;
  return rv;
}


bool ZSegmentationEncoderBitMap::contains(const ZIntPoint &point) const{
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  const ZIntPoint& pt = m_init_box.getFirstCorner();
  x -= pt.getX();
  y -= pt.getY();
  z -= pt.getZ();

  int width = m_init_box.getWidth();
  int height = m_init_box.getHeight();
  int slice = width * height;

  int index = z * slice + y * width + x;
  int out_index = index / (8*sizeof(int));
  int in_index = index % (8*sizeof(int));

  if(out_index >= (int)m_data.size()){
    return false;
  }
  return m_data[out_index] & (1 << in_index);
}


void ZSegmentationEncoderBitMap::labelStack(ZStack &stack, int value) const{
  int width = stack.width();
  int height = stack.height();
  int area = width*height;
  uint8_t* pdata = stack.array8();
  //int index = 0;

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();


  int min_y = getBoundBox().getFirstCorner().getY();
  int max_y = getBoundBox().getLastCorner().getY();
  int min_z = getBoundBox().getFirstCorner().getZ();
  int max_z = getBoundBox().getLastCorner().getZ();
  int min_x = getBoundBox().getFirstCorner().getX();
  int max_x = getBoundBox().getLastCorner().getX();

  for(int z = min_z; z <= max_z; ++z){
    for(int y = min_y; y <= max_y; ++y){
      for(int x = min_x; x <= max_x; ++x){
        if(ZSegmentationEncoder::contains(x,y,z)){
          pdata[(x-ofx) + (y-ofy)*width + (z-ofz)*area] = value;
        }
      }
    }
  }
}


double ZSegmentationEncoderBitMap::memUsage()const {
  double rv = m_data.size() * sizeof(int);
  return rv;
}


void ZSegmentationEncoderBitMap::unify(const ZSegmentationEncoder &segmentation_encoder){
  //pass
}


void ZSegmentationEncoderBitMap::_maybe_update_bound_box(int z, int y, int start, int end){
  m_maxz = m_maxz > z ? m_maxz : z;
  m_minz = m_minz < z ? m_minz : z;
  m_maxy = m_maxy > y ? m_maxy : y;
  m_miny = m_miny < y ? m_miny : y;
  m_maxx = m_maxx > end ? m_maxx : end;
  m_minx = m_minx < start ? m_minx : start;
}


ZSegmentationEncoderRaw::ZSegmentationEncoderRaw(){
  m_minx = INT32_MAX;
  m_maxx = 0;
  m_miny = INT32_MAX;
  m_maxy = 0;
  m_minz = INT32_MAX;
  m_maxz = 0;
  m_inited = false;
}


void ZSegmentationEncoderRaw::add(const ZIntPoint &point){
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  _maybe_update_bound_box(z,y,x,x);

  x -= m_init_box.getFirstCorner().getX();
  y -= m_init_box.getFirstCorner().getY();
  z -= m_init_box.getFirstCorner().getZ();

  int width = m_init_box.getWidth();
  int height = m_init_box.getHeight();
  int slice = width * height;

  int index = z * slice + y * width + x;

  if(!m_inited){
    m_data = std::shared_ptr<ZStack>(new ZStack(GREY,width,height,m_init_box.getDepth(),1));
    m_inited = true;
  }
  m_data->array8()[index] = 1;
}


void ZSegmentationEncoderRaw::consume(const ZStack &stack){
  initBoundBox(stack.getBoundBox());

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
      for(int w = 0; w < width; ++w){
        v = p[index++];
        if(v){
          add(ZIntPoint(w+ofx,h+ofy,d+ofz));
        }
      }
    }
  }
}


ZSegmentationEncoder* ZSegmentationEncoderRaw::clone()const{
  ZSegmentationEncoderRaw* rv = new ZSegmentationEncoderRaw();
  rv->m_data = shared_ptr<ZStack>(m_data->clone());
  return rv;
}


bool ZSegmentationEncoderRaw::contains(const ZIntPoint &point) const{
  int x = point.getX();
  int y = point.getY();
  int z = point.getZ();

  const ZIntPoint& pt = m_init_box.getFirstCorner();
  x -= pt.getX();
  y -= pt.getY();
  z -= pt.getZ();

  int width = m_init_box.getWidth();
  int height = m_init_box.getHeight();
  int slice = width * height;

  int index = z * slice + y * width + x;

  if(index >= (int)m_data->getVoxelNumber()){
    return false;
  }
  return m_data->array8()[index];
}


void ZSegmentationEncoderRaw::labelStack(ZStack &stack, int value) const{
  int width = stack.width();
  int height = stack.height();
  int area = width*height;
  uint8_t* pdata = stack.array8();
  //int index = 0;

  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();


  int min_y = getBoundBox().getFirstCorner().getY();
  int max_y = getBoundBox().getLastCorner().getY();
  int min_z = getBoundBox().getFirstCorner().getZ();
  int max_z = getBoundBox().getLastCorner().getZ();
  int min_x = getBoundBox().getFirstCorner().getX();
  int max_x = getBoundBox().getLastCorner().getX();

  for(int z = min_z; z <= max_z; ++z){
    for(int y = min_y; y <= max_y; ++y){
      for(int x = min_x; x <= max_x; ++x){
        if(ZSegmentationEncoder::contains(x,y,z)){
          pdata[(x-ofx) + (y-ofy)*width + (z-ofz)*area] = value;
        }
      }
    }
  }
}


double ZSegmentationEncoderRaw::memUsage()const {
  double rv = m_data->getVoxelNumber() * sizeof(uint8);
  //std::cout<<m_data->getVoxelNumber()<<std::endl;
  //std::cout<<m_data->width()<<m_data->height()<<m_data->depth()<<std::endl;
  //std::cout<<rv<<std::endl;
  return rv;
}


void ZSegmentationEncoderRaw::unify(const ZSegmentationEncoder &segmentation_encoder){
  //pass
}


void ZSegmentationEncoderRaw::_maybe_update_bound_box(int z, int y, int start, int end){
  m_maxz = m_maxz > z ? m_maxz : z;
  m_minz = m_minz < z ? m_minz : z;
  m_maxy = m_maxy > y ? m_maxy : y;
  m_miny = m_miny < y ? m_miny : y;
  m_maxx = m_maxx > end ? m_maxx : end;
  m_minx = m_minx < start ? m_minx : start;
}


void ZSegmentationEncoderScan::add(const ZIntPoint& point){
  m_data->addSegment(point.getZ(),point.getY(),point.getX(),point.getX());
}


bool ZSegmentationEncoderScan::contains(const ZIntPoint& point)const{
  return m_data->contains(point);
}


void ZSegmentationEncoderScan::unify(const ZSegmentationEncoder& segmentation_encoder){
//pass
}

void ZSegmentationEncoderScan::labelStack(ZStack& stack, int value)const{
    //m_data->labelStack(stack.c_stack(),value);
}


ZSegmentationEncoder* ZSegmentationEncoderScan::clone()const{
  //
}


double ZSegmentationEncoderScan::memUsage()const{
  return 0;
}


ZIntCuboid ZSegmentationEncoderScan::getBoundBox()const{
  return m_data->getBoundBox();
}


void ZSegmentationEncoderOctTree::add(const ZIntPoint &point){

}


bool ZSegmentationEncoderOctTree::contains(const ZIntPoint &point) const{
  if(m_data){
    return m_data->contains(point);
  }
  return false;
}


void ZSegmentationEncoderOctTree::consume(const ZStack &stack){
  m_data = shared_ptr<OctTree>(_construct(stack,stack.getBoundBox()));
}


ZSegmentationEncoderOctTree::OctTree* ZSegmentationEncoderOctTree::_construct(const ZStack &stack, const ZIntCuboid &box){

  /*int _x1=box.getFirstCorner().getX();
  int _y1=box.getFirstCorner().getY();
  int _z1=box.getFirstCorner().getZ();
  int _x2=box.getLastCorner().getX();
  int _y2=box.getLastCorner().getY();
  int _z2=box.getLastCorner().getZ();

  std::cout<<"("<<_x1<<","<<_y1<<","<<_z1<<")"<<"("<<_x2<<","<<_y2<<","<<_z2<<")"<<std::endl;*/

  int width = box.getWidth();
  int height = box.getHeight();
  int depth = box.getDepth();

  if(width <= 0 || height <= 0 || depth <=0){
    return nullptr;
  }

  int ofx = box.getFirstCorner().getX();
  int ofy = box.getFirstCorner().getY();
  int ofz = box.getFirstCorner().getZ();

  if(width == 1 && height == 1 && depth ==1){
    int v = stack.getIntValue(ofx,ofy,ofz);
    if(v){
      OctTree* rv = new OctTree();
      rv->leaf = true;
      rv->box = box;
      return rv;
    } else {
      return nullptr;
    }
  }

  OctTree* sub[8]={nullptr};

  for(int k = 0 ; k < 2; ++k){
    for(int j = 0; j < 2; ++j){
      for(int i = 0; i < 2; ++i){
        int x1 = ofx + i * width / 2;
        int y1 = ofy + j * height / 2;
        int z1 = ofz + k * depth / 2;
        int x2 = ofx + (i + 1) * width / 2 - 1;
        int y2 = ofy + (j + 1) * height / 2 - 1;
        int z2 = ofz + (k + 1) * depth / 2 - 1;
        sub[k*4+j*2+i] = _construct(stack,ZIntCuboid(x1,y1,z1,x2,y2,z2));
      }
    }
  }

  bool full_black = true;
  bool full_white = true;

  for(int i = 0; i < 8; ++i){
    if(sub[i]){
      full_white = false;
    } else {
      full_black = false;
    }
  }

  if(full_black){
    OctTree* rv = new OctTree();
    rv->box = box;
    bool leaf = true;
    for(int i = 0; i < 8; ++i){
      if(!sub[i]->leaf){
        leaf = false;
      }
    }
    if(leaf){
      rv->leaf = true;
      for(int i = 0; i < 8; ++i){
        delete sub[i];
      }
    } else {
      rv->leaf = false;
      for(int i = 0; i < 8; ++i){
        rv->child[i] = sub[i];
      }
    }
    return rv;
  } else if(full_white){
    return nullptr;
  } else {
    OctTree* rv = new OctTree();
    rv->box = box;
    rv->leaf = false;
    for(int i = 0; i < 8; ++i){
      rv->child[i] = sub[i];
    }
    return rv;
  }

  return nullptr;
}

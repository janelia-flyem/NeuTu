#include <iostream>
#include "zsegmentationscan.h"
#include "zstack.hxx"
#include "zobject3dscan.h"
#include "tz_constant.h"

//implementation for ZSegmentationScan

ZSegmentationScan::ZSegmentationScan()
{
  clearData();
}


void ZSegmentationScan::clearData()
{
  if(m_data.size() !=0 )
  {
    m_data.clear();
  }
  m_sz = MAX_INT32;
  m_ez = 0;
  m_sy = MAX_INT32;
  m_ey = 0;
  m_sx = MAX_INT32;
  m_ex = 0;
}


void ZSegmentationScan::unify(ZSegmentationScan *obj)
{

  int sz = std::min(minZ(), obj->minZ());
  int ez = std::max(maxZ(), obj->maxZ());
  int size_z = ez - sz + 1;

  int sy = std::min(minY(), obj->minY());
  int ey = std::max(maxY(), obj->maxY());
  int size_y = ey - sy + 1;

  int sx = std::min(minX(), obj->minX());
  int ex = std::max(maxX(), obj->maxX());

  //std::cout<<sx<<" "<<ex<<" "<<sy<<" "<<ey<<" "<<sz<<" "<<ez<<std::endl;
  std::vector<std::vector<std::vector<int>>> new_data;
  new_data.resize(size_z);
  for(int z = 0; z < size_z; ++z)
  {
    new_data[z].resize(size_y);
  }

  for(int z = 0; z < size_z; ++z)
  {
    for(int y = 0; y < size_y; ++y)
    {
      new_data[z][y] = canonize(getStrip(z+sz,y+sy),obj->getStrip(z+sz,y+sy));
    }
  }

  m_data = new_data;
  m_sz = sz;
  m_ez = ez;
  m_sy = sy;
  m_ey = ey;
  m_sx = sx;
  m_ex = ex;
}


std::vector<int> ZSegmentationScan::canonize(std::vector<int> &a, std::vector<int> &b)
{
  if(a.size() == 0)
  {
    return b;
  }
  if(b.size() == 0)
  {
    return a;
  }

  std::vector<int> rv;
  std::vector<int>::iterator itas = a.begin();
  std::vector<int>::iterator itbs = b.begin();
  std::vector<int>::iterator itae = itas + 1;
  std::vector<int>::iterator itbe = itbs + 1;

  while(itae < a.end() && itbe < b.end())
  {
    int a = *itas;
    int b = *itae;
    if(*itas < *itbs)
    {
      itas += 2;
      itae += 2;
    }
    else
    {
      a = *itbs;
      b = *itbe;
      itbs += 2;
      itbe += 2;
    }
    if(rv.size() == 0)
    {
      rv.push_back(a);
      rv.push_back(b);
    }
    else
    {
      int d = rv.back();
      rv.pop_back();
      if(a <= d)
      {
        if( b <= d)
        {
          rv.push_back(d);
        }
        else
        {
          rv.push_back(b);
        }
      }
      else
      {
        rv.push_back(d);
        rv.push_back(a);
        rv.push_back(b);
      }
    }
  }

  while(itae < a.end())
  {
    rv.push_back(*itas);
    rv.push_back(*itae);
    itas += 2;
    itae += 2;
  }
  while(itbe < b.end())
  {
    rv.push_back(*itbs);
    rv.push_back(*itbe);
    itbs += 2;
    itbe += 2;
  }
  return rv;
}


void ZSegmentationScan::preprareData(int depth, int height)
{
  //clearData();
  m_data.resize(depth);

  for(int i = 0; i < depth; ++i)
  {
    m_data[i].resize(height);
  }
}


ZIntCuboid ZSegmentationScan::getStackForegroundBoundBox(ZStack *stack)
{
  ZIntCuboid box(0,0,0,-1,-1,-1);
  if(!stack)
  {
    return box;
  }

  uint8_t *p = stack->array8();

  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();
  size_t area = width * height;

  int min_x = width, max_x = -1;
  int min_y = height, max_y = -1;
  int min_z = depth, max_z = -1;

  for(int z = 0 ; z < depth; ++z)
  {
    for(int y = 0; y < height; ++y)
    {
      uint8_t *q = p + z*area + y*width;
      for(int x = 0; x < width; ++x, ++q)
      {
        if(*q)
        {
          if(z < min_z) min_z = z;
          if(z > max_z) max_z = z;
          if(y < min_y) min_y = y;
          if(y > max_y) max_y = y;
          if(x < min_x) min_x = x;
          if(x > max_x) max_x = x;
        }
      }
    }
  }
  box.setFirstCorner(min_x,min_y,min_z);
  box.setLastCorner(max_x,max_y,max_z);
  box.translate(stack->getOffset());
  return box;
}

//int i = 1000;
void ZSegmentationScan::fromObject3DScan(ZObject3dScan *obj)
{
  ZIntCuboid box = obj->getBoundBox();
  ZStack * stack = new ZStack(GREY,box.getWidth(),box.getHeight(),box.getDepth(),1);
  stack->setOffset(obj->getBoundBox().getFirstCorner());
  obj->drawStack(stack,1);
  //stack->save((QString("/home/deli/")+QString::number(i++)+".tif").toStdString());
  fromStack(stack);
  delete stack;
}


ZObject3dScan* ZSegmentationScan::toObject3dScan()
{
  ZStack* stack = toStack();
  if(stack)
  {
    ZObject3dScan *rv = new ZObject3dScan();
    rv->loadStack(*stack);
    //stack->save("/home/deli/stack1.tif");
    delete stack;
    return rv;
  }
  return NULL;
}


ZStack* ZSegmentationScan::toStack()
{
  if(m_data.size() == 0)
  {
    return NULL;
  }
  ZIntCuboid box = getBoundBox();
  ZStack* rv = new ZStack(GREY, box.getWidth(), box.getHeight(), box.getDepth(), 1);
  rv->setOffset(box.getFirstCorner());
  labelStack(rv);
  return rv;
}


void ZSegmentationScan::fromStack(ZStack* stack)
{
  if(!stack)
  {
    return ;
  }
  clearData();
  int width = stack->width();
  int height = stack->height();
  size_t area  = width*height;

  ZIntCuboid box = getStackForegroundBoundBox(stack);

  if(box.getDepth() <= 0 || box.getHeight() <=0 || box.getWidth() <=0)
  {
    return ;
  }

  int ofx = stack->getOffset().m_x;
  int ofy = stack->getOffset().m_y;
  int ofz = stack->getOffset().m_z;
  m_offset = stack->getOffset();

  m_sz = box.getFirstCorner().m_z;
  m_sy = box.getFirstCorner().m_y;
  m_sx = box.getFirstCorner().m_x;
  m_ez = box.getLastCorner().m_z;
  m_ey = box.getLastCorner().m_y;
  m_ex = box.getLastCorner().m_x;

  preprareData(box.getDepth(), box.getHeight());

  uint8_t* p = stack->array8();

  for(int k = m_sz; k <= m_ez; ++k)
  {
    for(int j = m_sy; j <= m_ey; ++j)
    {
      uint8_t * q = p + (k-ofz)*area + (j-ofy)*width + (m_sx-ofx);
      int sx = -1, ex = -1;
      int flag = 0;
      for(int i = m_sx; i <= m_ex; ++i, ++q)
      {
        if(*q && (flag == 0))
        {
          sx = i;
          flag = 1;
        }
        else if((!*q) && (flag ==1))
        {
          ex = i -1;
          flag = 0;
          addSegment(k,j,sx,ex);
        }
      }
      if(flag ==1)
      {
        addSegment(k,j,sx,m_ex);
      }
    }
  }
}


void ZSegmentationScan::labelStack(ZStack *stack, int v)
{
  if(!stack)
  {
    return ;
  }

  uint8_t *p = stack->array8();
  int width = stack->width();
  int height = stack->height();
  int area = width * height;

  int ofx = stack->getOffset().m_x;//m_offset.m_x;
  int ofy = stack->getOffset().m_y;//m_offset.m_y;
  int ofz = stack->getOffset().m_z;//m_offset.m_z;

  for(uint z = 0; z < m_data.size(); ++z)
  {
    std::vector<std::vector<int>>& dy = m_data[z];
    for(uint y = 0; y < dy.size(); ++y)
    {
      std::vector<int>& dx = dy[y];
      uint8_t *q = p + (z + m_sz - ofz)*area + (y + m_sy - ofy)*width;
      for(uint x = 0; x < dx.size(); x += 2)
      {
        int sx = dx[x] - ofx;
        int ex = dx[x+1] - ofx;
        uint8_t* pd = q + sx;
        for(int i = sx; i <= ex; ++i)
        {
          *pd++ = v;
        }
      }
    }
  }
}


void ZSegmentationScan::maskStack(ZStack *stack)
{
  if(!stack)
  {
    return ;
  }

  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();

  ZStack* label = new ZStack(GREY, width, height, depth, 1);
  label->setOffset(stack->getOffset());
  labelStack(label);

  uint8_t *p = label->array8();
  uint8_t *pend = p + label->getVoxelNumber();
  uint8_t *q = stack->array8();

  for(; p != pend; ++p, ++q)
  {
    *q = (*q) * (*p);
  }

  delete label;
}


void ZSegmentationScan::addSegment(int z, int y, int sx, int ex)
{
  getStrip(z,y).push_back(sx);
  getStrip(z,y).push_back(ex);
}


void ZSegmentationScan::translate(ZIntPoint offset)
{
  translate(offset.getX(), offset.getY(), offset.getZ());
}


void ZSegmentationScan::translate(int ofx, int ofy, int ofz)
{
  for(uint z = 0; z < m_data.size(); ++z)
  {
    std::vector<std::vector<int>>& slice = m_data[z];
    for(uint y = 0; y < slice.size(); ++y)
    {
      std::vector<int>& strip = slice[y];
      for(uint x = 0; x < strip.size(); ++x)
      {
        strip[x] += ofx;
      }
    }
  }
  m_sx += ofx;
  m_ex += ofx;
  m_sy += ofy;
  m_ey += ofy;
  m_sz += ofz;
  m_ez += ofz;
}

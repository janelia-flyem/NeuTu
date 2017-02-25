#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
#include "zstackdoc.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"

ZStack* recover2OriginalSize(ZStack* original,ZStack* sampled,int step)
{
  int width=original->width();
  int height=original->height();
  int depth=original->depth();
  ZStack* recover=new ZStack(original->kind(),width,height,depth,original->channelNumber());
  uint8_t* pr=recover->array8();
  uint8_t* ps=sampled->array8();
  int s_width=sampled->width();
  int s_slice=s_width*sampled->height();
  size_t s_offset=0;
  int r_slice=width*height;

  for(int k=0;k<sampled->depth();++k)
  {
    s_offset=k*s_slice;
    for(int j=0;j<sampled->height();++j)
    {
      size_t ss_offset=s_offset+j*s_width;
      for(int i=0;i<s_width;++i)
      {
        uint8_t v=ps[ss_offset+i];
        for(int d=k*step;d<std::min((k+1)*step,depth);++d)
          for(int h=j*step;h<std::min((j+1)*step,height);++h)
            for(int w=i*step;w<std::min((i+1)*step,width);++w)
              pr[d*r_slice+h*width+w]=v;

      }
    }
  }
  return recover;
}



ZStack* getEdgeMap(const ZStack& stack)
{
  int index=1;
  uchar index_map[256][256]={0};
  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::makeZeroStack(
        stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);

  size_t area = C_Stack::area(originalStack);
  size_t offset = 0;
  uint8_t *originalArray = originalStack->array;
  uint8_t *maskArray= maskStack->array;
  //x scan
  for (int z = 0; z < depth; ++z)
  {
    for (int y = 0; y < height; ++y)
    {
      offset++;
      for (int x = 1; x < width; ++x)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - 1];
        if(v>t)
        {
          uint8_t tem=v;
          v=t;
          t=tem;
        }
        if (v<t)
        {
          if(index_map[v][t])
          {
            maskArray[offset] = maskArray[offset-1]=index_map[v][t];

          }
          else
          {
            maskArray[offset] =maskArray[offset-1]= index_map[v][t]=index++;
          }
        }
        offset++;
      }
    }
  }


  //y scan
  for (int z = 0; z < depth; ++z)
  {
    for (int x = 0; x < width; ++x)
    {
      offset = area * z + x + width;
      for (int y = 1; y < height; ++y)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - width];
        if(v>t)
        {
          uint8_t tem=v;
          v=t;
          t=tem;
        }
        if (v<t)
        {
          if(index_map[v][t])
          {
            maskArray[offset] = maskArray[offset-width]=index_map[v][t];

          }
          else
          {
            maskArray[offset] =maskArray[offset-width]= index_map[v][t]=index++;
          }
        }
        offset += width;
      }
    }
  }

  //z scan
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      offset = width * y + x + area;
      for (int z = 1; z < depth; ++z)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - area];
        if(v>t)
        {
          uint8_t tem=v;
          v=t;
          t=tem;
        }
        if (v<t)
        {
          if(index_map[v][t])
          {
            maskArray[offset] = maskArray[offset-area]=index_map[v][t];

          }
          else
          {
            maskArray[offset] =maskArray[offset-area]= index_map[v][t]=index++;
          }
        }
        offset += area;
      }
    }
  }

  return mask;
}


ZStack* getBoundBoxes(ZStack* stack,std::vector<ZIntCuboid>&boxes)
{
  ZStack* rv=getEdgeMap(*stack);
  std::vector<ZObject3dScan*> edges =ZObject3dScan::extractAllObject(*rv);
  for(std::vector<ZObject3dScan*>::iterator it=edges.begin();it!=edges.end();++it)
  {
    boxes.push_back((*it)->getBoundBox());
    delete *it;
  }
  return rv;
}


inline bool hasNeighborOnEdge(const ZStack* edge_map,int x,int y,int z)
{
  int width=edge_map->width();
  int height=edge_map->height();
  int depth=edge_map->depth();
  int slice=width*height;
  const uint8_t* p=edge_map->array8();
  for(int k=std::max(0,z-1);k<=std::min(depth-1,z+1);++k)
  {
    for(int j=std::max(0,y-1);j<=std::min(height-1,y+1);++j)
    {
      for(int i=std::max(0,x-1);i<=std::min(width-1,x+1);++i)
      {
        if(p[i+j*width+k*slice])
        {
          return true;
        }
      }
    }
  }
  return false;
}



ZStack* generateSeeds(ZStack* src,const ZIntCuboid& box,const ZStack* edge_map,
                                 ZStack *seed_map,
                                 const ZStack* stack,int step)
{
  int width=stack->width(),height=stack->height(),slice=height*width;
  int o_w=src->width(),o_h=src->height(),o_d=src->depth();

  int start_x=box.getFirstCorner().m_x;
  int start_y=box.getFirstCorner().m_y;
  int start_z=box.getFirstCorner().m_z;
  int end_x=box.getLastCorner().m_x;
  int end_y=box.getLastCorner().m_y;
  int end_z=box.getLastCorner().m_z;

  int max_x=std::min((end_x+1)*step,o_w);
  int max_y=std::min((end_y+1)*step,o_h);
  int max_z=std::min((end_z+1)*step,o_d);

  int seed_w=max_x-start_x*step;
  int seed_h=max_y-start_y*step;
  int seed_d=max_z-start_z*step;
  int seed_s=seed_w*seed_h;

  int seed_x=start_x*step;
  int seed_y=start_y*step;
  int seed_z=start_z*step;
  ZStack* seed=new ZStack(GREY,seed_w,seed_h,seed_d,1);
  seed->setOffset(seed_x+src->getOffset().m_x,seed_y+src->getOffset().m_y,seed_z+src->getOffset().m_z);

#define ADD_SEED(x,y,z,offset)\
  {\
  psd[offset]=1;\
  int v=ps[offset];\
  for(int k=z*step-seed_z;k<std::min((z+1)*step,max_z)-seed_z;++k)\
  for(int j=y*step-seed_y;j<std::min((y+1)*step,max_y)-seed_y;++j)\
  for(int i=x*step-seed_x;i<std::min((x+1)*step,max_x)-seed_x;++i)\
    pseed[i+j*seed_w+k*seed_s]=v;}\

  const uint8_t* ps=stack->array8();
  uint8_t* psd=seed_map->array8();
  const uint8_t* p=edge_map->array8();
  uint8_t* pseed=seed->array8();
  //x scan
  int offset=0,_offset=0;
  for(int z=start_z;z<=end_z;++z)
  {
    offset=z*slice;
    for(int y=start_y;y<=end_y;++y)
    {
      _offset=start_x+1+y*width+offset;
      for(int x=start_x+1;x<=end_x-1;++x,++_offset)
      {
        if(!p[_offset]&&(p[_offset-1] || p[_offset+1]))
        {
          ADD_SEED(x,y,z,_offset);
        }
      }
      if(start_x!=end_x)
      {
        if(!p[start_x+y*width+offset]&&p[start_x+1+y*width+offset])
        {
          ADD_SEED(start_x,y,z,start_x+y*width+offset);
        }
        if(!p[end_x+y*width+offset]&&p[end_x-1+y*width+offset])
        {
          ADD_SEED(end_x,y,z,end_x+y*width+offset);
        }
      }
    }
  }
  //y scan
  for(int z=start_z;z<=end_z;++z)
  {
    offset=z*slice;
    for(int x=start_x;x<=end_x;++x)
    {
      _offset=offset+x+(start_y+1)*width;
      for(int y=start_y+1;y<=end_y-1;++y,_offset+=width)
      {
        if(!p[_offset]&&(p[_offset-width] || p[_offset+width]))//is a edge point
        {
          ADD_SEED(x,y,z,_offset);
        }
      }
      if(start_y!=end_y)
      {
        if(!p[offset+x+start_y*width]&&p[offset+x+(start_y+1)*width])
        {
          ADD_SEED(x,start_y,z,offset+x+start_y*width);
        }
        if(!p[end_y*width+offset+x]&&p[(end_y-1)*width+offset+x])
        {
          ADD_SEED(x,end_y,z,end_y*width+offset+x);
        }
      }
    }
  }
  //z scan
  for(int y=start_y;y<=end_y;++y)
  {
    offset=y*width;
    for(int x=start_x;x<=end_x;++x)
    {
      _offset=offset+x+(start_z+1)*slice;
      for(int z=start_z+1;z<=end_z-1;++z,_offset+=slice)
      {
        if(!p[_offset]&&(p[_offset-slice] || p[_offset+slice]))
        {
          ADD_SEED(x,y,z,_offset);
        }
      }
      if(start_z!=end_z)
      {
        if(!p[start_z*slice+offset+x]&&p[(start_z+1)*slice+offset+x])
        {
          ADD_SEED(x,y,start_z,start_z*slice+offset+x);
        }
        if(!p[end_z*slice+offset+x]&&p[(end_z-1)*slice+offset+x])
        {
          ADD_SEED(x,y,end_z,end_z*slice+offset+x);
        }
      }
    }
  }
  return seed;
#undef ADD_SEED
}



void setNoneEdgePoints2Zero(ZStack* stack,const ZStack* edge_map,const ZStack* seed_map,
                            const Cuboid_I& range,int step)
{
  int width=stack->width(),height=stack->height(),depth=stack->depth(),slice=width*height;
  int e_w=edge_map->width(),e_s=edge_map->height()*e_w;
  uint8_t *q=stack->array8();
  const uint8_t *s=edge_map->array8(),*t=seed_map->array8();

  int start_z=range.cb[2]/step,start_y=range.cb[1]/step,start_x=range.cb[0]/step;
  int end_z=range.ce[2]/step,end_y=range.ce[1]/step,end_x=range.ce[0]/step;

  for(int z=start_z;z<=end_z;++z)
  {
    int offset=z*e_s;
    for(int y=start_y;y<=end_y;++y)
    {
      int _offset=offset+y*e_w+start_x;
      for(int x=start_x;x<=end_x;++x,++_offset)
      {
        if((!s[_offset])&&(!t[_offset]))
        {
          for(int d=z*step;d<std::min((z+1)*step,depth);++d)
            for(int h=y*step;h<std::min((y+1)*step,height);++h)
              for(int w=x*step;w<std::min((x+1)*step,width);++w)
              {
                q[d*slice+h*width+w]=0;
              }
        }
      }
    }
  }
}


void localWaterShed(ZStack* seed,Cuboid_I& range,
                    ZStack* recovered,ZStack* original,const ZStack* edge_map,
                    const ZStack* seed_map,int step)
{
  int x1=range.cb[0],x2=range.ce[0];
  int y1=range.cb[1],y2=range.ce[1];
  int z1=range.cb[2],z2=range.ce[2];
  ZStackWatershed watershed;
  //set points that are (not seeds and not at any edge) to zero
  setNoneEdgePoints2Zero(original,edge_map,seed_map,range,step);

  range.cb[0]+=original->getOffset().m_x;
  range.ce[0]+=original->getOffset().m_x;
  range.cb[1]+=original->getOffset().m_y;
  range.ce[1]+=original->getOffset().m_y;
  range.cb[2]+=original->getOffset().m_z;
  range.ce[2]+=original->getOffset().m_z;
  watershed.setRange(range);
  watershed.setFloodingZero(false);

  ZStack* result=watershed.run(original,seed);
  if(!result)
  {
    std::cout<<"local watershed failed"<<std::endl;
    return;
  }

  //merge result into recovered image
  uint8_t* p=recovered->array8();
  uint8_t* q=result->array8();
  int width=original->width();
  int slice=width*original->height();

  int r_w=result->width(),r_s=result->height()*r_w;

  int rofx=result->getOffset().m_x-original->getOffset().m_x,
      rofy=result->getOffset().m_y-original->getOffset().m_y,
      rofz=result->getOffset().m_z-original->getOffset().m_z;

  int off,_off;
  for(int z=z1-rofz;z<=z2-rofz;++z)
  {
    off=z*r_s;
    for(int y=y1-rofy;y<=y2-rofy;++y)
    {
      _off=off+y*r_w+x1-rofx;
      for(int x=x1-rofx;x<=x2-rofx;++x,++_off)
      {
        int v=q[_off];
        if(v)p[x+rofx+(y+rofy)*width+(z+rofz)*slice]=v;
      }
    }
  }
  delete result;
}


Cuboid_I getRange(const ZIntCuboid& box,const ZStack* stack,int step)
{
  Cuboid_I range;
  const ZIntPoint&p=box.getFirstCorner();
  const ZIntPoint&q=box.getLastCorner();

  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();

  range.cb[0]=p.m_x*step;
  range.cb[1]=p.m_y*step;
  range.cb[2]=p.m_z*step;
  range.ce[0]=std::min((q.m_x+1)*step-1,width-1);
  range.ce[1]=std::min((q.m_y+1)*step-1,height-1);
  range.ce[2]=std::min((q.m_z+1)*step-1,depth-1);

  return range;
}


ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}


ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src)
{
  ZStack* recovered=recover2OriginalSize(src,sampled_watershed,_scale);
  //ZStack* recovered=new ZStack(src->kind(),src->width(),src->height(),src->depth(),1);
  recovered->setOffset(src->getOffset());
  //return recovered;
  std::vector<ZIntCuboid>boxes;
  //get bound boxes of each each
  ZStack* edge_map=getBoundBoxes(sampled_watershed,boxes);
  ZStack* seed_map=new ZStack(GREY,sampled_watershed->width(),sampled_watershed->height(),
                              sampled_watershed->depth(),1);

  //process each bound box
  for(std::vector<ZIntCuboid>::iterator it=boxes.begin();it!=boxes.end();++it)
  {
    const ZIntCuboid& box=*it;
    Cuboid_I range=getRange(box,src,_scale);
    ZStack* seed=generateSeeds(src,box,edge_map,seed_map,sampled_watershed,_scale);

    localWaterShed(seed,range,recovered,src,edge_map,seed_map,_scale);
    delete seed;
  }

  delete edge_map;
  delete seed_map;
  return recovered;

}


ZStack* ZStackMultiScaleWatershed::run(ZStack *src,QList<ZSwcTree*>& trees,int scale)
{
  _scale=scale;
  ZStack* rv=0;
  std::vector<ZStack*> seeds;
  getSeeds(seeds,trees);
  //run watershed
  ZStackWatershed watershed;
  if(scale==1)
  {
    rv=watershed.run(src,seeds);
    for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
    {
      delete *it;
    }
    return rv;
  }
  //down sample src stack
  ZStack* sampled=src->clone();
  sampled->downsampleMinIgnoreZero(scale-1,scale-1,scale-1);
  //sampled->downsampleMin(scale-1,scale-1,scale-1);
  ZStack* sampled_watershed=watershed.run(sampled,seeds);

  if(sampled_watershed)
  {
    ZStack* src_clone=src->clone();
    rv=upSampleAndRecoverEdge(sampled_watershed,src_clone);
    delete sampled_watershed;
    delete src_clone;
  }
  delete sampled;
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    delete *it;
  }
  return rv;
}


void ZStackMultiScaleWatershed::getSeeds(std::vector<ZStack*>& seeds,QList<ZSwcTree*>& trees)
{
  if(trees.size()<2 || trees.size()>255)
  {
    return ;
  }
  uint seed_index=1;

  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree* tree=*it;
    ZCuboid box=tree->getBoundBox();
    ZStack* seed=new ZStack(GREY,std::max(1,(int)(box.width()/_scale)),
                            std::max(1,(int)(box.height()/_scale)),
                            std::max(1,(int)(box.depth()/_scale)),1);
    ZPoint f=box.firstCorner();
    ZPoint l=box.lastCorner();
    ZIntPoint off((f.getX()+l.getX())/2/_scale,
                  (f.getY()+l.getY())/2/_scale,
                  (f.getZ()+l.getZ())/2/_scale);
    seed->setOffset(off);
    uint8_t* p=seed->array8();
    for(uint i=0;i<seed->getVoxelNumber();++i)
    {
      *p++=seed_index;
    }
    seed_index++;
    seeds.push_back(seed);
  }
}

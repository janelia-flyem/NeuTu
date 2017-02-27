#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
#include "zstackdoc.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"

ZStack* upSample(int width,int height,int depth,int scale,ZStack* sampled)
{
  int s_w=sampled->width(),s_s=s_w*sampled->height();
  ZStack* recover=new ZStack(GREY,width,height,depth,1);
  uint8_t* pr=recover->array8(),*ps_begin=sampled->array8(),*ps=0;
  int strip=width/scale,remain=width%scale;
  for(int k=0;k<depth;++k)
  {
    for(int j=0;j<height;++j)
    {
      ps=ps_begin+j/scale*s_w+k/scale*s_s;
      for(int i=0;i<strip;++i)
      {
        for(int t=0;t<scale;++t)
        {
          *pr++=*ps;
        }
        ++ps;
      }
      for(int i=0;i<remain;++i)
      {
        *pr++=*ps;
      }
    }
  }
  return recover;
}


void _getEdgeMap(uchar index_map[256][256],int& index,uint8_t* originalArray,uint8_t* maskArray,
                  int width,size_t area,
                  int start_x,int end_x,int start_y,int end_y,int start_z,int end_z)
{
  size_t offset=0;
  for (int z = start_z; z <= end_z; ++z)
  {
    for (int y = start_y; y <= end_y; ++y)
    {
      offset=z*area+y*width+start_x+1;
      for (int x = start_x+1; x <= end_x; ++x,++offset)
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
      }
    }
  }

  for (int z = start_z; z <=end_z; ++z)
  {
    for (int x = start_x; x <= end_x; ++x)
    {
      offset = area * z + x + (start_y+1)*width;
      for (int y = start_y+1; y <= end_y; ++y,offset += width)
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
      }
    }
  }

  for (int y = start_y; y <= end_y; ++y)
  {
    for (int x = start_x; x <=end_x; ++x)
    {
      offset = width * y + x + (start_z+1)*area;
      for (int z = start_z+1; z <= end_z; ++z,offset += area)
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
      }
    }
  }

}

ZStack* getEdgeMap(const ZStack& stack)
{
  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::makeZeroStack(
        stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);

  uint8_t *originalArray = originalStack->array;
  uint8_t *maskArray= maskStack->array;
  int index=1;
  uchar index_map[256][256]={0};

  int start_x,end_x,start_y,end_y,start_z,end_z;
  int slice_z=depth>180?5:(depth>120?4:(depth>60?3:(depth>20?2:1)));
  int slice_y=height>180?5:(height>120?4:(height>60?3:(height>20?2:1)));
  int slice_x=width>180?5:(width>120?4:(width>60?3:(width>20?2:1)));
  for(start_z=0;start_z<depth;start_z+=depth/slice_z)
  {
    end_z=start_z+depth/slice_z>=depth?depth-1:start_z+depth/slice_z;
    for(start_y=0;start_y<height;start_y+=height/slice_y)
    {
      end_y=start_y+height/slice_y>=height?height-1:start_y+height/slice_y;
      for(start_x=0;start_x<width;start_x+=width/slice_x)
      {
        assert(index<256);
        end_x=start_x+width/slice_x>=width?width-1:start_x+width/slice_x;
        _getEdgeMap(index_map,index,originalArray,maskArray,width,width*height,
                    start_x,end_x,start_y,end_y,start_z,end_z);
        memset(index_map,0,256*256);
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


ZStack* generateSeeds(ZStack* src,const Cuboid_I& range,const ZStack* edge_map,
                                 ZStack *seed_map,
                                 const ZStack* stack,int step)
{
  int width=src->width(),height=src->height(),slice=height*width;
  int s_w=stack->width(),s_h=stack->height(),s_s=s_w*s_h;

  int start_x=range.cb[0];
  int start_y=range.cb[1];
  int start_z=range.cb[2];
  int end_x=range.ce[0];
  int end_y=range.ce[1];
  int end_z=range.ce[2];

  int seed_w=end_x-start_x+1;
  int seed_h=end_y-start_y+1;
  int seed_d=end_z-start_z+1;
  int seed_s=seed_w*seed_h;

  ZStack* seed=new ZStack(GREY,seed_w,seed_h,seed_d,1);
  seed->setOffset(start_x+src->getOffset().m_x,start_y+src->getOffset().m_y,start_z+src->getOffset().m_z);

#define ADD_SEED(x,y,z,offset)\
  {\
  psd[offset]=1;\
  pseed[x-start_x+(y-start_y)*seed_w+(z-start_z)*seed_s]=ps[x/step+y/step*s_w+z/step*s_s];}

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


void localWaterShed(ZStack* seed,Cuboid_I& range,
                    ZStack* recovered,ZStack* original,const ZStack* edge_map,
                    const ZStack* seed_map)
{
  int x1=range.cb[0],x2=range.ce[0];
  int y1=range.cb[1],y2=range.ce[1];
  int z1=range.cb[2],z2=range.ce[2];
  uint8_t* p_rec_begin=recovered->array8(),*p_src=original->array8(),*p_rec=0;
  const uint8_t *p_edg=edge_map->array8(),*p_sed=seed_map->array8();

  int width=original->width();
  int slice=width*original->height();
  int off,_off;
  for(int z=z1;z<=z2;++z)
  {
    off=z*slice;
    for(int y=y1;y<=y2;++y)
    {
      _off=off+y*width+x1;
      for(int x=x1;x<=x2;++x,++_off)
      {
        if((!p_edg[_off])&&(!p_sed[_off]))
        {
           p_src[_off]=0;
        }
      }
    }
  }
  ZStackWatershed watershed;
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

  uint8_t* p_res=result->array8();
  //merge result into recovered image
  int r_w=result->width(),r_h=result->height(),r_d=result->depth();
  int rofx=result->getOffset().m_x-original->getOffset().m_x,
      rofy=result->getOffset().m_y-original->getOffset().m_y,
      rofz=result->getOffset().m_z-original->getOffset().m_z;
  for(int z=0;z<r_d;++z)
  {
    for(int y=0;y<r_h;++y)
    {
      p_rec=p_rec_begin+(z+rofz)*slice+(y+rofy)*width+rofx;
      for(int x=0;x<r_w;++x)
      {
        if(*p_res)
        {
          *p_rec=*p_res;
        }
        ++p_rec,++p_res;
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
  range.cb[0]=std::max(0,(p.m_x-1)*step-1);
  range.cb[1]=std::max(0,(p.m_y-1)*step-1);
  range.cb[2]=std::max(0,(p.m_z-1)*step-1);
  range.ce[0]=std::min((q.m_x+2)*step,width-1);
  range.ce[1]=std::min((q.m_y+2)*step,height-1);
  range.ce[2]=std::min((q.m_z+2)*step,depth-1);

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
  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),_scale,sampled_watershed);
  recovered->setOffset(src->getOffset());
  std::vector<ZIntCuboid>boxes;
  //get bound boxes of each each
  ZStack* _edge_map=getBoundBoxes(sampled_watershed,boxes);
  ZStack* edge_map=upSample(src->width(),src->height(),src->depth(),_scale,_edge_map);
  ZStack* seed_map=new ZStack(GREY,src->width(),src->height(),src->depth(),1);
  //process each bound box
  for(std::vector<ZIntCuboid>::iterator it=boxes.begin();it!=boxes.end();++it)
  {
    const ZIntCuboid& box=*it;
    Cuboid_I range=getRange(box,src,_scale);
    ZStack* seed=generateSeeds(src,range,edge_map,seed_map,sampled_watershed,_scale);
    localWaterShed(seed,range,recovered,src,edge_map,seed_map);
    delete seed;
  }
  delete _edge_map;
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

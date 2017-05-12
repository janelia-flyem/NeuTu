#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
//#include "zstackdoc.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"
#include "zintcuboid.h"
#include "zcuboid.h"
#include "zswctree.h"

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


ZStack* getEdgeMap(const ZStack& stack)
{
  int index=1;
  unsigned char index_map[256][256]={0};

  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::MakeZeroStack(
        stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  uint8_t *originalArray = originalStack->array;
  uint8_t *maskArray= maskStack->array;
  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);
  int area=width*height;

  size_t offset=0;

  for (int z = 0; z <= depth-1; ++z)
  {
    for (int y = 0; y <= height-1; ++y)
    {
      offset=z*area+y*width+1;
      for (int x = 1; x <= width-1; ++x,++offset)
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

  for (int z = 0; z <=depth-1; ++z)
  {
    for (int x = 0; x <= width-1; ++x)
    {
      offset = area * z + x + width;
      for (int y = 1; y <= height-1; ++y,offset += width)
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

  for (int y = 0; y <= height-1; ++y)
  {
    for (int x = 0; x <=width-1; ++x)
    {
      offset = width * y + x + area;
      for (int z = 1; z <= depth-1; ++z,offset += area)
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
  return mask;
}


ZStack* getBoundBoxes(ZStack* stack,std::vector<ZIntCuboid>&boxes)
{
  ZStack* rv=getEdgeMap(*stack);
  size_t off=0,width=rv->width(),height=rv->height(),depth=rv->depth();
  uint min_x[256]={MAX_INT32},max_x[256]={0},min_y[256]={MAX_INT32},
      max_y[256]={0},min_z[256]={MAX_INT32},max_z[256]={0};
  int max=0;
  uint8_t* p=rv->array8();
  for(uint z=0;z<depth;++z)
    for(uint y=0;y<height;++y)
      for(uint x=0;x<width;++x,++off)
      {
        int t=p[off];
        if(t)
        {
          if(t>max)max=t;
          if(x<min_x[t])min_x[t]=x;
          if(y<min_y[t])min_y[t]=y;
          if(z<min_z[t])min_z[t]=z;
          if(x>max_x[t])max_x[t]=x;
          if(y>max_y[t])max_y[t]=y;
          if(z>max_z[t])max_z[t]=z;
        }
      }
  std::vector<Cuboid_I> cubs;
  for(int i=1;i<=max;++i)
  {
    Cuboid_I c;
    c.cb[0]=min_x[i],c.cb[1]=min_y[i],c.cb[2]=min_z[i];
    c.ce[0]=max_x[i],c.ce[1]=max_y[i],c.ce[2]=max_z[i];
    cubs.push_back(c);
  }
  for(uint i=0;i<cubs.size();++i)
  {
    bool flag=true;
    for(uint j=0;j<cubs.size();++j)
    {
      if(i!=j && Cuboid_I_Overlap_Volume(&cubs[i],&cubs[j])==Cuboid_I_Volume(&cubs[i]))
      {
        flag=false;
        break;
      }
    }
    if(flag)
      boxes.push_back(ZIntCuboid(cubs[i]));
  }
  /*for(int i=1;i<=max;++i)
  {
    boxes.push_back(ZIntCuboid(min_x[i],min_y[i],min_z[i],max_x[i],max_y[i],max_z[i]));
  }*/
  return rv;
}


ZStack* generateSeeds(ZStack* src,const ZIntCuboid& box,const ZStack* edge_map,const ZStack* stack,int step)
{
  int width=src->width(),height=src->height(),depth=src->depth();
  int s_w=stack->width(),s_h=stack->height(),s_s=s_w*s_h;

  int start_x=box.getFirstCorner().m_x,start_x_r=start_x*step;
  int start_y=box.getFirstCorner().m_y,start_y_r=start_y*step;
  int start_z=box.getFirstCorner().m_z,start_z_r=start_z*step;
  int end_x=box.getLastCorner().m_x;
  int end_y=box.getLastCorner().m_y;
  int end_z=box.getLastCorner().m_z;


  int seed_w=std::min(width,(end_x+1)*step)-start_x_r;
  int seed_h=std::min(height,(end_y+1)*step)-start_y_r;
  int seed_d=std::min(depth,(end_z+1)*step)-start_z_r;
  int seed_s=seed_w*seed_h;

  ZStack* seed=0;

  const uint8_t* ps=stack->array8();
  const uint8_t* p=edge_map->array8();
  uint8_t* pseed=0;

#define ADD_SEED(sa,ea,sb,eb,sc,ta,tb,tc,v)\
  {\
    if(!seed)\
    {\
      seed=new ZStack(GREY,seed_w,seed_h,seed_d,1);\
      seed->setOffset(start_x_r+src->getOffset().m_x,\
                      start_y_r+src->getOffset().m_y,\
                      start_z_r+src->getOffset().m_z);\
      pseed=seed->array8();\
    }\
    for(int a=(sa);a<(ea);++a)\
      for(int b=(sb);b<(eb);++b)\
        pseed[a*(ta)+b*(tb)+(sc)*(tc)]=v;\
  }

  //x scan
  int offset=0,_offset=0;
  for(int z=start_z;z<=end_z;++z)
  {
    offset=z*s_s;
    for(int y=start_y;y<=end_y;++y)
    {
      _offset=start_x+1+y*s_w+offset;
      for(int x=start_x+1;x<=end_x-1;++x,++_offset)
      {
        if(!p[_offset])
        {
          if(p[_offset-1])
          {
            ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,y*step-start_y_r,
                     std::min((y+1)*step,height)-start_y_r,x*step-start_x_r,seed_s,seed_w,1,ps[_offset]);
          }
          if(p[_offset+1])
          {
            ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,y*step-start_y_r,
                     std::min((y+1)*step,height)-start_y_r,x*step+step-1-start_x_r,seed_s,seed_w,1,ps[_offset]);
          }
        }
      }
      if(start_x!=end_x)
      {
        if(!p[start_x+y*s_w+offset]&&p[start_x+1+y*s_w+offset])
        {
          ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,y*step-start_y_r,
                   std::min((y+1)*step,height)-start_y_r,start_x*step+step-1-start_x_r,seed_s,seed_w,1,ps[_offset]);
        }
        if(!p[end_x+y*s_w+offset]&&p[end_x-1+y*s_w+offset])
        {
          ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,y*step-start_y_r,
                   std::min((y+1)*step,height)-start_y_r,end_x*step-start_x_r,seed_s,seed_w,1,ps[_offset]);
        }
      }
    }
  }
  //y scan
  for(int z=start_z;z<=end_z;++z)
  {
    offset=z*s_s;
    for(int x=start_x;x<=end_x;++x)
    {
      _offset=offset+x+(start_y+1)*s_w;
      for(int y=start_y+1;y<=end_y-1;++y,_offset+=s_w)
      {
        if(!p[_offset])
        {
          if(p[_offset-s_w])
          {
            ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,x*step-start_x_r,
                     std::min((x+1)*step,width)-start_x_r,
                     y*step-start_y_r,seed_s,1,seed_w,ps[_offset]);
          }
          if(p[_offset+s_w])
          {
            ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,x*step-start_x_r,
                     std::min((x+1)*step,width)-start_x_r,
                     y*step+step-1-start_y_r,seed_s,1,seed_w,ps[_offset]);
          }
        }
      }
      if(start_y!=end_y)
      {
        if(!p[offset+x+start_y*s_w]&&p[offset+x+(start_y+1)*s_w])
        {
          ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,x*step-start_x_r,
                   std::min((x+1)*step,width)-start_x_r,
                   start_y*step+step-1-start_y_r,seed_s,1,seed_w,ps[_offset]);
        }
        if(!p[end_y*s_w+offset+x]&&p[(end_y-1)*s_w+offset+x])
        {
          ADD_SEED(z*step-start_z_r,std::min((z+1)*step,depth)-start_z_r,x*step-start_x_r,
                   std::min((x+1)*step,width)-start_x_r,
                   end_y*step-start_y_r,seed_s,1,seed_w,ps[_offset]);
        }
      }
    }
  }
  //z scan
  for(int y=start_y;y<=end_y;++y)
  {
    offset=y*s_w;
    for(int x=start_x;x<=end_x;++x)
    {
      _offset=offset+x+(start_z+1)*s_s;
      for(int z=start_z+1;z<=end_z-1;++z,_offset+=s_s)
      {
        if(!p[_offset])
        {
          if(p[_offset-s_s])
          {
            ADD_SEED(y*step-start_y_r,std::min((y+1)*step,height)-start_y_r,x*step-start_x_r,
                     std::min((x+1)*step,width)-start_x_r,
                     z*step-start_z_r,seed_w,1,seed_s,ps[_offset]);
          }
          if(p[_offset+s_s])
          {
            ADD_SEED(y*step-start_y_r,std::min((y+1)*step,height)-start_y_r,x*step-start_x_r,
                     std::min((x+1)*step,width)-start_x_r,
                     z*step+step-1-start_z_r,seed_w,1,seed_s,ps[_offset]);
          }
        }
      }
      if(start_z!=end_z)
      {
        if(!p[start_z*s_s+offset+x]&&p[(start_z+1)*s_s+offset+x])
        {
          ADD_SEED(y*step-start_y_r,std::min((y+1)*step,height)-start_y_r,x*step-start_x_r,
                   std::min((x+1)*step,width)-start_x_r,
                   start_z*step+step-1-start_z_r,seed_w,1,seed_s,ps[_offset]);
        }
        if(!p[end_z*s_s+offset+x]&&p[(end_z-1)*s_s+offset+x])
        {
          ADD_SEED(y*step-start_y_r,std::min((y+1)*step,height)-start_y_r,x*step-start_x_r,
                   std::min((x+1)*step,width)-start_x_r,
                   end_z*step-start_z_r,seed_w,1,seed_s,ps[_offset]);
        }
      }
    }
  }
  return seed;
#undef ADD_SEED
}


void localWaterShed(ZStack* seed,Cuboid_I& range,
                    ZStack* recovered,ZStack* original,ZStack* edge_map,int scale)
{
  int x1=range.cb[0],x2=range.ce[0];
  int y1=range.cb[1],y2=range.ce[1];
  int z1=range.cb[2],z2=range.ce[2];
  uint8_t* p_rec_begin=recovered->array8(),*p_src=original->array8(),*p_rec=0;
  uint8_t *p_edg=edge_map->array8(),*p_seed=seed->array8();

  int width=original->width(),e_w=edge_map->width(),s_w=seed->width();
  int slice=width*original->height(),e_s=e_w*edge_map->height(),s_s=seed->height()*s_w;
  int off,_off,off_s,_off_s,off_r,_off_r;
  for(int z=z1;z<=z2;++z)
  {
    off=z/scale*e_s;
    off_s=(z-z1)*s_s;
    off_r=z*slice;
    for(int y=y1;y<=y2;++y)
    {
      _off=off+y/scale*e_w;
      _off_s=off_s+(y-y1)*s_w;
      _off_r=off_r+y*width;
      for(int x=x1;x<=x2;++x,++_off_s,++_off_r)
        if(!p_edg[_off+x/scale]&&!p_seed[_off_s])
            p_src[_off_r]=0;
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
  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),_scale,sampled_watershed);
  recovered->setOffset(src->getOffset());
  std::vector<ZIntCuboid>boxes;
  //get bound boxes of each each
  ZStack* edge_map=getBoundBoxes(sampled_watershed,boxes);
  uint8_t* p=edge_map->array8();
  size_t off=0;
  uint width=edge_map->width(),slice=edge_map->height()*width;
  //process each bound box
  for(uint i=0;i<boxes.size();++i)
  {
    const ZIntCuboid& box=boxes[i];
    Cuboid_I range=getRange(box,src,_scale);
    ZStack* seed=generateSeeds(src,box,edge_map,sampled_watershed,_scale);
    if(seed)
    {
      localWaterShed(seed,range,recovered,src,edge_map,_scale);
      if(i!=boxes.size()-1)
      {
        for(int z=box.getFirstCorner().m_z;z<=box.getLastCorner().m_z;++z)
        {
          for(int y=box.getFirstCorner().m_y;y<=box.getLastCorner().m_y;++y)
          {
            off=z*slice+y*width;
            for(int x=box.getFirstCorner().m_x;x<=box.getLastCorner().m_x;++x)
               p[off++]=0;
          }
        }
      }
      delete seed;
    }
  }
  delete edge_map;
  return recovered;

}

#if defined(_QT_GUI_USED_)
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
#endif

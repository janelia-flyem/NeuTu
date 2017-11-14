#include <fstream>
#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
//#include "zstackdoc.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"
#include "zswcforest.h"
#include "zintcuboid.h"
#include "zcuboid.h"
#include "zswctree.h"
#include "zobject3d.h"
#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE

#if defined(_ENABLE_SURFRECON_)
#include "surfrecon.h"
#endif


ZStack* ZStackMultiScaleWatershed::upSample(int width,int height,int depth,ZStack* sampled)
{
  int scale=m_scale;
  int s_w=sampled->width(),s_h=sampled->height(),s_s=s_w*s_h;
  ZStack* recover=new ZStack(GREY,width,height,depth,1);
  uint8_t* src=sampled->array8(),*dst=recover->array8();

  int xcnt=0,ycnt=0,zcnt=0;

  for (int z = 0; z < depth; ++z)
  {
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        *dst++=*src;
        if(++xcnt>=scale)
        {
          xcnt=0;
          ++src;
        }
      }
      if(xcnt)
      {
        xcnt=0;
        ++src;
      }
      if(++ycnt>=scale)
      {
        ycnt=0;
      }
      else
      {
        src-=s_w;
      }
    }
    if(ycnt)
    {
      ycnt=0;
      src+=s_w;
    }
    if(++zcnt>=scale)
    {
      zcnt=0;
    }
    else
    {
      src-=s_s;
    }
  }
  return recover;
}


ZStack* ZStackMultiScaleWatershed::getEdgeMap(const ZStack& stack)
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


void ZStackMultiScaleWatershed::generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* edge_map,const ZStack* stack)
{
  int step=m_scale;
  int slice=width*height;
  int s_w=stack->width(),s_h=stack->height(),s_d=stack->depth(),s_s=s_w*s_h;

  const uint8_t* ps=stack->array8();
  const uint8_t* p=edge_map->array8();
  uint8_t* pseed=seed->array8();

#define ADD_SEED(sa,ea,sb,eb,sc,ta,tb,tc,v)\
  {\
    for(int a=(sa);a<(ea);++a)\
      for(int b=(sb);b<(eb);++b)\
        pseed[a*(ta)+b*(tb)+(sc)*(tc)]=v;\
  }
  //x scan
  int offset=0,_offset=0;
  for(int z=0;z<=s_d;++z)
  {
    offset=z*s_s;
    for(int y=0;y<=s_h;++y)
    {
      _offset=1+y*s_w+offset;
      for(int x=1;x<s_w-1;++x,++_offset)
      {
        if(!p[_offset])
        {
          if(p[_offset-1])
          {
            ADD_SEED(z*step,std::min((z+1)*step,depth),y*step,
                     std::min((y+1)*step,height),x*step,slice,width,1,ps[_offset]);
          }
          if(p[_offset+1])
          {
            ADD_SEED(z*step,std::min((z+1)*step,depth),y*step,
                     std::min((y+1)*step,height),x*step+step-1,slice,width,1,ps[_offset]);
          }
        }
      }
      if(s_w>1)
      {
        if(!p[y*s_w+offset]&&p[1+y*s_w+offset])
        {
          ADD_SEED(z*step,std::min((z+1)*step,depth),y*step,
                   std::min((y+1)*step,height),step-1,slice,width,1,ps[y*s_w+offset]);
        }
        if(!p[s_w-1+y*s_w+offset]&&p[s_w-2+y*s_w+offset])
        {
          ADD_SEED(z*step,std::min((z+1)*step,depth),y*step,
                   std::min((y+1)*step,height),(s_w-1)*step,slice,width,1,ps[s_w-1+y*s_w+offset]);
        }
      }
    }
  }
  //y scan
  for(int z=0;z<=s_d;++z)
  {
    offset=z*s_s;
    for(int x=0;x<=s_w;++x)
    {
      _offset=offset+x+s_w;
      for(int y=1;y<s_h-1;++y,_offset+=s_w)
      {
        if(!p[_offset])
        {
          if(p[_offset-s_w])
          {
            ADD_SEED(z*step,std::min((z+1)*step,depth),x*step,
                     std::min((x+1)*step,width),
                     y*step,slice,1,width,ps[_offset]);
          }
          if(p[_offset+s_w])
          {
            ADD_SEED(z*step,std::min((z+1)*step,depth),x*step,
                     std::min((x+1)*step,width),
                     y*step+step-1,slice,1,width,ps[_offset]);
          }
        }
      }
      if(s_h>1)
      {
        if(!p[offset+x]&&p[offset+x+s_w])
        {
          ADD_SEED(z*step,std::min((z+1)*step,depth),x*step,
                   std::min((x+1)*step,width),
                   step-1,slice,1,width,ps[offset+x]);
        }
        if(!p[(s_h-1)*s_w+offset+x]&&p[(s_h-2)*s_w+offset+x])
        {
          ADD_SEED(z*step,std::min((z+1)*step,depth),x*step,
                   std::min((x+1)*step,width),
                   (s_h-1)*step,slice,1,width,ps[(s_h-1)*s_w+offset+x]);
        }
      }
    }
  }
  //z scan
  for(int y=0;y<=s_h;++y)
  {
    offset=y*s_w;
    for(int x=0;x<=s_w;++x)
    {
      _offset=offset+x+s_s;
      for(int z=1;z<s_d-1;++z,_offset+=s_s)
      {
        if(!p[_offset])
        {
          if(p[_offset-s_s])
          {
            ADD_SEED(y*step,std::min((y+1)*step,height),x*step,
                     std::min((x+1)*step,width),
                     z*step,width,1,slice,ps[_offset]);
          }
          if(p[_offset+s_s])
          {
            ADD_SEED(y*step,std::min((y+1)*step,height),x*step,
                     std::min((x+1)*step,width),
                     z*step+step-1,width,1,slice,ps[_offset]);
          }
        }
      }
      if(s_d>1)
      {
        if(!p[offset+x]&&p[s_s+offset+x])
        {
          ADD_SEED(y*step,std::min((y+1)*step,height),x*step,
                   std::min((x+1)*step,width),
                   step-1,width,1,slice,ps[offset+x]);
        }
        if(!p[(s_d-1)*s_s+offset+x]&&p[(s_d-2)*s_s+offset+x])
        {
          ADD_SEED(y*step,std::min((y+1)*step,height),x*step,
                   std::min((x+1)*step,width),
                   (s_d-1)*step,width,1,slice,ps[(s_d-1)*s_s+offset+x]);
        }
      }
    }
  }
#undef ADD_SEED
}


ZStack* ZStackMultiScaleWatershed::labelAreaNeedUpdate(ZStack* edge_map,ZStack* seed,ZStack* srcStack)
{
  int scale=m_scale;
  int width=seed->width(),height=seed->height(),depth=seed->depth();
  int s_w=edge_map->width(),s_h=edge_map->height(),s_s=s_w*s_h;

  ZStack* rv=new ZStack(GREY,width,height,depth,1);
  uint8_t* map=edge_map->array8(),*dst=rv->array8(),*sd=seed->array8();
  uint8_t *src=srcStack?srcStack->array8():NULL;

  int xcnt=0,ycnt=0,zcnt=0;
  double cnt=0;

  for (int z = 0; z < depth; ++z)
  {
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        if(*map || *sd)
        {
          *dst=srcStack?(*src):1;
          cnt+=1;
        }
        ++dst,++src,++sd;
        if(++xcnt>=scale)
        {
          xcnt=0;
          ++map;
        }
      }
      if(xcnt)
      {
        xcnt=0;
        ++map;
      }
      if(++ycnt>=scale)
      {
        ycnt=0;
      }
      else
      {
        map-=s_w;
      }
    }
    if(ycnt)
    {
      ycnt=0;
      map+=s_w;
    }
    if(++zcnt>=scale)
    {
      zcnt=0;
    }
    else
    {
      map-=s_s;
    }
  }
  return rv;
}


ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}


ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src)
{
  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),sampled_watershed);
  recovered->setOffset(src->getOffset());

  ZStack* edge_map=getEdgeMap(*sampled_watershed);
  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);
  generateSeeds(seed,src->width(),src->height(),src->depth(),edge_map,sampled_watershed);

  ZStack* src_clone=labelAreaNeedUpdate(edge_map,seed,src);

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);
  ZStack* result=watershed.run(src_clone,seed);
  if(!result){
    std::cout<<"local watershed failed"<<std::endl;
    return recovered;
  }

  uint8_t* pres=result->array8(),*psrc=src->array8(),*prec=recovered->array8(),*pend=pres+result->getVoxelNumber();
  for(;pend!=pres;++pres,++prec,++psrc){
     if(*pres)*prec=*pres;
     if(!*psrc)*prec=0;
  }

  delete seed;
  delete result;
  delete edge_map;
  delete src_clone;
  return recovered;

}


#if defined(_QT_GUI_USED_)
ZStack* ZStackMultiScaleWatershed::run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale)
{
  m_scale=scale;
  ZStack* rv=NULL;
  ZStack* seed=NULL;
  ZStackWatershed watershed;
  if(m_scale==1){
    seed=toSeedStack(seeds,src->width(),src->height(),src->depth(),src->getOffset());
    seed->setOffset(src->getOffset());
    rv=watershed.run(src,seed);
    delete seed;
    return rv;
  }
  //down sample src stack
  ZStack* sampled=src->clone();
  sampled->downsampleMinIgnoreZero(m_scale-1,m_scale-1,m_scale-1);
  seed=toSeedStack(seeds,sampled->width(),sampled->height(),sampled->depth(),sampled->getOffset());
  ZStack* sampled_watershed=watershed.run(sampled,seed);
  if(sampled_watershed){
    rv=upSampleAndRecoverEdge(sampled_watershed,src);
    delete sampled_watershed;
  }

  delete sampled;
  delete seed;
  return rv;
}


ZStack* ZStackMultiScaleWatershed::toSeedStack(std::vector<ZObject3d*>& seeds,int width,int height,int depth,ZIntPoint offset)
{
  ZStack* mask=new ZStack(GREY,width,height,depth,1);
  mask->setOffset(offset);
  for(auto seed:seeds){
      uint8_t label = seed->getLabel();
      uint8_t *array = mask->array8();
      size_t area = width*height;
      for (size_t i = 0; i < seed->size(); ++i){
        int x = seed->getX(i);
        int y = seed->getY(i);
        int z = seed->getZ(i);
        x /= m_scale;
        y /= m_scale;
        z /= m_scale;
        x -= offset.getX();
        y -= offset.getY();
        z -= offset.getZ();
        array[z * area + y * width + x] = label;
      }
  }
  return mask;
}
#endif

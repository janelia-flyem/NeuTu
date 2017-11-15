#include <fstream>
#include <QProcess>
#include <QStringList>
#include <QFile>
#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"
#include "zswcforest.h"
#include "zintcuboid.h"
#include "zcuboid.h"
#include "zswctree.h"
#include "zobject3d.h"
#include "zintcuboid.h"
#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE

#if defined(_ENABLE_SURFRECON_)
#include "surfrecon.h"
#endif

//Upsample a stack into with*height*depth
ZStack* ZStackMultiScaleWatershed::upSample(int width,int height,int depth,ZStack* stack)
{
  int scale=m_scale;
  int s_width=stack->width();
  int s_height=stack->height();
  int s_slice=s_width*s_height;

  ZStack* recovered=new ZStack(GREY,width,height,depth,1);
  uint8_t* src=stack->array8();
  uint8_t* dst=recovered->array8();

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
        src-=s_width;
      }
    }
    if(ycnt)
    {
      ycnt=0;
      src+=s_width;
    }
    if(++zcnt>=scale)
    {
      zcnt=0;
    }
    else
    {
      src-=s_slice;
    }
  }
  return recovered;
}


//find the boundaries of objects in the stack, boundaries between different objects have different labels
ZStack* ZStackMultiScaleWatershed::getBoundaryMap(const ZStack& stack)
{
  int index=1;
  unsigned char index_map[256][256]={0};

  const Stack *originalStack = stack.c_stack();
  ZStack *mask = ZStackFactory::MakeZeroStack(stack.width(), stack.height(), stack.depth());
  Stack *maskStack = mask->c_stack();

  uint8_t *originalArray = originalStack->array;
  uint8_t *maskArray= maskStack->array;
  int width = C_Stack::width(originalStack);
  int height = C_Stack::height(originalStack);
  int depth = C_Stack::depth(originalStack);
  int slice=width*height;

  size_t offset=0;

  for (int z = 0; z <= depth-1; ++z)//x scan
  {
    for (int y = 0; y <= height-1; ++y)
    {
      offset=z*slice+y*width+1;
      for (int x = 1; x <= width-1; ++x,++offset)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - 1];
        if(v==0||t==0||v==t)
            continue;
        if(v>t)
        {
          std::swap(v,t);
        }
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

  for (int z = 0; z <=depth-1; ++z)//y scan
  {
    for (int x = 0; x <= width-1; ++x)
    {
      offset = slice * z + x + width;
      for (int y = 1; y <= height-1; ++y,offset += width)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - width];
        if(v==0||t==0||v==t)
            continue;
        if(v>t)
        {
          std::swap(v,t);
        }

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

  for (int y = 0; y <= height-1; ++y)//z scan
  {
    for (int x = 0; x <=width-1; ++x)
    {
      offset = width * y + x + slice;
      for (int z = 1; z <= depth-1; ++z,offset += slice)
      {
        uint8_t v=originalArray[offset],t=originalArray[offset - slice];
        if(v==0||t==0||v==t)
            continue;
        if(v>t)
        {
          std::swap(v,t);
        }
        if(index_map[v][t])
        {
          maskArray[offset] = maskArray[offset-slice]=index_map[v][t];
        }
        else
        {
          maskArray[offset] =maskArray[offset-slice]= index_map[v][t]=index++;
        }
      }
    }
  }
  return mask;
}


//generate seeds for second pass segmentation
void ZStackMultiScaleWatershed::generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* boundary_map,const ZStack* stack)
{
  int step=m_scale;
  int slice=width*height;
  int s_width=stack->width();
  int s_height=stack->height();
  int s_depth=stack->depth();
  size_t s_slice=s_width*s_height;

  const uint8_t* pstack=stack->array8();
  const uint8_t* pboundary=boundary_map->array8();
  uint8_t* pseed=seed->array8();

  int offset=0,_offset=0;

  for(int z=0;z<=s_depth-1;++z)  //x scan
  {
    offset=z*s_slice;
    for(int y=0;y<=s_height-1;++y)
    {
      _offset=1+y*s_width+offset;
      for(int x=1;x<s_width-1;++x,++_offset)
      {
        if(!pboundary[_offset])// current voxel is not on the boundary
        {
          if(pboundary[_offset-1])// left voxel is on the boundary
          {
            addSeedX(seed,x*step,y*step,std::min(height,(y+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
          if(pboundary[_offset+1])//right voxel is on the boundary
          {
            addSeedX(seed,x*step+step-1,y*step,std::min(height,(y+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
        }
      }
    }
  }

  for(int z=0;z<=s_depth-1;++z)  //y scan
  {
    offset=z*s_slice;
    for(int x=0;x<=s_width-1;++x)
    {
      _offset=offset+x+s_width;
      for(int y=1;y<s_height-1;++y,_offset+=s_width)
      {
        if(!pboundary[_offset])
        {
          if(pboundary[_offset-s_width])
          {
            addSeedY(seed,y*step,x*step,std::min(width,(x+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
          if(pboundary[_offset+s_width])
          {
            addSeedY(seed,y*step+step-1,x*step,std::min(width,(x+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
        }
      }
    }
  }

  //z scan
  for(int y=0;y<=s_height-1;++y)
  {
    offset=y*s_width;
    for(int x=0;x<=s_width-1;++x)
    {
      _offset=offset+x+s_slice;
      for(int z=1;z<s_depth-1;++z,_offset+=s_slice)
      {
        if(!pboundary[_offset])
        {
          if(pboundary[_offset-s_slice])
          {
            addSeedZ(seed,z*step,x*step,std::min(width,(x+1)*step),y*step,std::min((y+1)*step,height),pstack[_offset]);
          }
          if(pboundary[_offset+s_slice])
          {
            addSeedZ(seed,z*step+step-1,x*step,std::min(width,(x+1)*step),y*step,std::min((y+1)*step,height),pstack[_offset]);
          }
        }
      }
    }
  }
}


//label area needed second pass segmentation: seed union boundary;  and return the bounding box
ZStack* ZStackMultiScaleWatershed::labelAreaNeedUpdate(ZStack* boundary_map,ZStack* seed,ZIntCuboid& boundbox,ZStack* srcStack)
{
  int scale=m_scale;
  int width=seed->width();
  int height=seed->height();
  int depth=seed->depth();
  int s_width=boundary_map->width();
  int s_height=boundary_map->height();
  size_t s_slice=s_width*s_height;

  ZStack* rv=new ZStack(GREY,width,height,depth,1);
  uint8_t* pmap=boundary_map->array8();
  uint8_t* pdst=rv->array8();
  uint8_t* pseed=seed->array8();
  uint8_t* psrc=srcStack?srcStack->array8():NULL;

  int xcnt=0,ycnt=0,zcnt=0;
  double cnt=0;

  int min_x=MAX_INT32,min_y=MAX_INT32,min_z=MAX_INT32;
  int max_x=0,max_y=0,max_z=0;

  for (int z = 0; z < depth; ++z)
  {
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        if(*pmap || *pseed)//if current voxel is seed or boundary,it needs update
        {
          *pdst=srcStack?(*psrc):1;
          cnt+=1;
          if(x<min_x)min_x=x;
          if(x>max_x)max_x=x;
          if(y<min_y)min_y=y;
          if(y>max_y)max_y=y;
          if(z<min_z)min_z=z;
          if(z>max_z)max_z=z;
        }
        ++pdst,++psrc,++pseed;
        if(++xcnt>=scale)
        {
          xcnt=0;
          ++pmap;
        }
      }
      if(xcnt)
      {
        xcnt=0;
        ++pmap;
      }
      if(++ycnt>=scale)
      {
        ycnt=0;
      }
      else
      {
        pmap-=s_width;
      }
    }
    if(ycnt)
    {
      ycnt=0;
      pmap+=s_width;
    }
    if(++zcnt>=scale)
    {
      zcnt=0;
    }
    else
    {
      pmap-=s_slice;
    }
  }
  boundbox.set(min_x,min_y,min_z,max_x,max_y,max_z);
  return rv;
}


ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}


ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverBoundary(ZStack* sampled_watershed,ZStack* src)
{

  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),sampled_watershed);
  recovered->setOffset(src->getOffset());

  ZStack* boundary_map=getBoundaryMap(*sampled_watershed);
  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);

  generateSeeds(seed,src->width(),src->height(),src->depth(),boundary_map,sampled_watershed);
  ZIntCuboid box;
  ZStack* src_clone=labelAreaNeedUpdate(boundary_map,seed,box,src);

  src_clone->crop(box);
  seed->crop(box);

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);
  ZStack* result=watershed.run(src_clone,seed);
  if(!result){
    std::cout<<"local watershed failed"<<std::endl;
    return recovered;
  }

  int s_x=box.getFirstCorner().getX();
  int s_y=box.getFirstCorner().getY();
  int s_z=box.getFirstCorner().getZ();
  int width=src->width(),height=src->height();
  size_t slice=width*height;

  uint8_t *pres=result->array8();
  uint8_t *prec=recovered->array8();

  for(int k=0;k<result->depth();k++){//update second pass segmenttaion result into result
    for(int j=0;j<result->height();++j){
      for(int i=0;i<result->width();++i,++pres){
          if(*pres){
              prec[(i+s_x)+(j+s_y)*width+(k+s_z)*slice]=*pres;
          }
      }
    }
  }

  uint8_t *psrc=src->array8();
  uint8_t *pend=psrc+src->getVoxelNumber();

  for(;pend!=psrc;++prec,++psrc){//if a voxel in the original stack is background, it should be background in the result
     if(!*psrc)*prec=0;
  }

  delete seed;
  delete result;
  delete boundary_map;
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
    rv=upSampleAndRecoverBoundary(sampled_watershed,src);
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



void ZStackMultiScaleWatershed::addSeedX(ZStack* pSeed,int x,int sy,int ey,int sz,int ez,uint8_t v){
  uint8_t* p=pSeed->array8();
  int width=pSeed->width();
  int slice=pSeed->height()*width;
  for(int y=sy;y<ey;++y){
    for(int z=sz;z<ez;++z){
      p[x+width*y+slice*z]=v;
    }
  }
}

void ZStackMultiScaleWatershed::addSeedY(ZStack* pSeed,int y,int sx,int ex,int sz,int ez,uint8_t v){
  uint8_t* p=pSeed->array8();
  int width=pSeed->width();
  int slice=pSeed->height()*width;
  for(int x=sx;x<ex;++x){
    for(int z=sz;z<ez;++z){
      p[x+width*y+slice*z]=v;
    }
  }
}

void ZStackMultiScaleWatershed::addSeedZ(ZStack* pSeed,int z,int sx,int ex,int sy,int ey,uint8_t v){
  uint8_t* p=pSeed->array8();
  int width=pSeed->width();
  int slice=pSeed->height()*width;
  for(int x=sx;x<ex;++x){
    for(int y=sy;y<ey;++y){
      p[x+width*y+slice*z]=v;
    }
  }
}

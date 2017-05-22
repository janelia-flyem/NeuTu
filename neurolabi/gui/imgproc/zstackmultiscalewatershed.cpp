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

#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE

#if defined(_ENABLE_SURFRECON_)
#include "surfrecon.h"
#endif

/*
void printStack(ZStack* s)//for test
{
  for(int j=0;j<s->height();++j)
  {
    for(int i=0;i<s->width();++i)
    {
      std::cout<<(int)s->array8()[i+j*s->width()]<<" ";
    }
    std::cout<<std::endl;
  }
  std::cout<<std::endl;
}


void loadStack(ZStack*& img,std::vector<ZStack*>& seeds)
{
  std::ifstream fin;
  fin.open("/home/deli/img.txt");
  int width,height;
  fin>>height>>width;
  img=new ZStack(GREY,width,height,1,1);
  for(int j=0;j<height;++j)
  {
    for(int i=0;i<width;++i)
    {
      fin>>img->array8()[i+j*width];
      img->array8()[i+j*width]-='0';
    }
  }
  int ns,x,y;
  fin>>ns;
  for(int i=0;i<ns;++i)
  {
    ZStack* seed=new ZStack(GREY,1,1,1,1);
    fin>>y>>x;
    fin>>seed->array8()[0];
    seed->array8()[0]-='0';
    seed->setOffset(x,y,0);
    seeds.push_back(seed);
  }
}
*/

ZStack* ZStackMultiScaleWatershed::upSample(int width,int height,int depth,ZStack* sampled)
{
  int scale=_scale;
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
/*
  ZStack *rv = ZStackFactory::makeZeroStack(
  stack.width(), stack.height(), stack.depth());

  uint8_t *src=mask->array8(),*dst=rv->array8();

  for(int z=0;z<depth;++z)
  {
    bool z0=z>0?true:false;
    bool z1=z<depth-1?true:false;
    for(int y=0;y<height;++y)
    {
      bool y0=y>0?true:false;
      bool y1=y<height-1?true:false;
      for(int x=0;x<width;++x,++src,++dst)
      {
        if(*src)
        {
          *dst=*src;
          if(x>0)*(dst-1)=*src;
          if(x<width-1)*(dst+1)=*src;
          if(y0)*(dst-width)=*src;
          if(y1)*(dst+width)=*src;
          if(z0)*(dst-area)=*src;
          if(z1)*(dst+area)=*src;
        }
      }
    }
  }
  delete mask;
  return rv;*/
}

/*
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
  //for(int i=1;i<=max;++i)
  //{
  //  boxes.push_back(ZIntCuboid(min_x[i],min_y[i],min_z[i],max_x[i],max_y[i],max_z[i]));
  //}
  return rv;
}*/


void ZStackMultiScaleWatershed::generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* edge_map,const ZStack* stack)
{
  int step=_scale;
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
  int scale=_scale;
  int width=seed->width(),height=seed->height(),depth=seed->depth();
  int s_w=edge_map->width(),s_h=edge_map->height(),s_s=s_w*s_h;

  ZStack* rv=new ZStack(GREY,width,height,depth,1);
  uint8_t* map=edge_map->array8(),*dst=rv->array8(),*sd=seed->array8();
  uint8_t *src=srcStack?srcStack->array8():nullptr;

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

  std::cout<<"rough edge and seeds cnts:"<<cnt/seed->getVoxelNumber()<<std::endl;

  return rv;
}

/*
void localWaterShed(ZStack* seed,ZStack* recovered,ZStack* src,ZStack* edge_map,int scale)
{
  ZStack* src_clone=extractNoneEdgeAndSeedPoints(edge_map,seed,src,scale);

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);
  ZStack* result=watershed.run(src_clone,seed);
  if(!result)
  {
    std::cout<<"local watershed failed"<<std::endl;
    return;
  }

  uint8_t* pres=result->array8(),*prec=recovered->array8();

  int depth=src->depth(),width=src->width(),height=src->height();

  for(int z=0;z<depth;++z)
  {
    for(int y=0;y<height;++y)
    {
      for(int x=0;x<width;++x,++pres,++prec)
      {
        if(*pres)*prec=*pres;
      }
    }
  }
  delete result;
}*/


/*
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
}*/


ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}


ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src)
{
  clock_t start,end;
  start=clock();
  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),sampled_watershed);
  end=clock();
  std::cout<<"upsampling time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;
  //std::cout<<"recovered:"<<std::endl;
  //printStack(recovered);

  recovered->setOffset(src->getOffset());

  ZStack* edge_map=getEdgeMap(*sampled_watershed);

  //std::cout<<"edge_map:"<<std::endl;
  //printStack(edge_map);

  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);
  //seed->setOffset(src->getOffset());

  start=clock();
  generateSeeds(seed,src->width(),src->height(),src->depth(),edge_map,sampled_watershed);
  end=clock();
  std::cout<<"seeds generating time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;
  //std::cout<<"seed:"<<std::endl;
  //std::cout<<seed->getOffset().m_x<<" "<<seed->getOffset().m_y<<std::endl;
  //printStack(seed);
  start=clock();
  ZStack* src_clone=labelAreaNeedUpdate(edge_map,seed,src);
  end=clock();
  std::cout<<"find rough edge points time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  //return src_clone;

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);

  start=clock();
  ZStack* result=watershed.run(src_clone,seed);
  end=clock();
  std::cout<<"local watershed time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  if(!result)
  {
    std::cout<<"local watershed failed"<<std::endl;
    return recovered;
  }


  uint8_t* pres=result->array8(),*psrc=src->array8(),*prec=recovered->array8(),*pend=pres+result->getVoxelNumber();

  start=clock();
  for(;pend!=pres;++pres,++prec,++psrc)
  {
     if(*pres)*prec=*pres;
     if(!*psrc)*prec=0;
  }
  end=clock();
  std::cout<<"update edge time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  delete seed;
  delete result;
  delete edge_map;
  delete src_clone;
  return recovered;

}
/*
void ZStackMultiScaleWatershed::test()
{
    ZStack *src;
    std::vector<ZStack *> seeds;
    loadStack(src,seeds);
    ZStack* rv=0;
    std::cout<<"src:"<<std::endl;
    printStack(src);

    rv=ZStackWatershed().run(src,seeds);
    std::cout<<"result:"<<std::endl;
    printStack(rv);


    ZStack* sampled=src->clone();
    sampled->downsampleMinIgnoreZero(1,1,1);
    std::cout<<"sampled:"<<std::endl;
    printStack(sampled);
    std::cout<<"seeds:"<<std::endl;
    for(int i=0;i<seeds.size();++i)
    {
       ZStack* seed=seeds[i];
       seed->setOffset(seed->getOffset().m_x/2,seed->getOffset().m_y/2,0);
       std::cout<<seed->getOffset().m_y<<" "<<seed->getOffset().m_x<<std::endl;
    }

    ZStack* sampled_watershed=ZStackWatershed().run(sampled,seeds);
    std::cout<<"sampled result:"<<std::endl;
    printStack(sampled_watershed);

<<<<<<< HEAD
    _scale=2;
    if(sampled_watershed)
    {
      ZStack* src_clone=src->clone();
      rv=upSampleAndRecoverEdge(sampled_watershed,src_clone);
      std::cout<<"result:"<<std::endl;
      printStack(rv);
    }


    sampled->downsampleMinIgnoreZero(1,1,1);
    std::cout<<"sampled:"<<std::endl;
    printStack(sampled);
    std::cout<<"seeds:"<<std::endl;
    for(int i=0;i<seeds.size();++i)
    {
       ZStack* seed=seeds[i];
       seed->setOffset(seed->getOffset().m_x/2,seed->getOffset().m_y/2,0);
       std::cout<<seed->getOffset().m_y<<" "<<seed->getOffset().m_x<<std::endl;
    }

    sampled_watershed=ZStackWatershed().run(sampled,seeds);
    std::cout<<"sampled result:"<<std::endl;
    printStack(sampled_watershed);

    _scale=4;
    if(sampled_watershed)
    {
      ZStack* src_clone=src->clone();
      rv=upSampleAndRecoverEdge(sampled_watershed,src_clone);
      std::cout<<"result:"<<std::endl;
      printStack(rv);
    }


    sampled->downsampleMinIgnoreZero(1,1,1);
    std::cout<<"sampled:"<<std::endl;
    printStack(sampled);
    std::cout<<"seeds:"<<std::endl;
    for(int i=0;i<seeds.size();++i)
    {
       ZStack* seed=seeds[i];
       seed->setOffset(seed->getOffset().m_x/2,seed->getOffset().m_y/2,0);
       std::cout<<seed->getOffset().m_y<<" "<<seed->getOffset().m_x<<std::endl;
    }

   sampled_watershed=ZStackWatershed().run(sampled,seeds);
    std::cout<<"sampled result:"<<std::endl;
    printStack(sampled_watershed);

    _scale=8;
    if(sampled_watershed)
    {
      ZStack* src_clone=src->clone();
      rv=upSampleAndRecoverEdge(sampled_watershed,src_clone);
      std::cout<<"result:"<<std::endl;
      printStack(rv);
    }

}
*/

#if defined(_QT_GUI_USED_)
ZStack* ZStackMultiScaleWatershed::run(ZStack *src,QList<ZSwcTree*>& trees,int scale)
{
  _scale=scale;
  ZStack* rv=0;
  //std::vector<ZStack*> seeds;
  ZStack* seed=new ZStack(GREY,std::max(1.0,src->width()/_scale),
                          std::max(1.0,src->height()/_scale),std::max(1.0,src->depth()/_scale),1);
  fillSeed(seed,trees,ZIntPoint(src->getOffset().m_x/_scale,src->getOffset().m_y/_scale,src->getOffset().m_z/_scale));
  //run watershed

  ZStackWatershed watershed;
  if(scale==1)
  {
    rv=watershed.run(src,seed);
    delete seed;
    return rv;
  }

  //down sample src stack
  ZStack* sampled=src->clone();
  clock_t start,end;
  start=clock();
  sampled->downsampleMinIgnoreZero(scale-1,scale-1,scale-1);
  end=clock();
  std::cout<<"downsampling time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  //sampled->downsampleMin(scale-1,scale-1,scale-1);
  start=clock();
  ZStack* sampled_watershed=watershed.run(sampled,seed);
  end=clock();
  std::cout<<"downsampled image segmentation time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  if(sampled_watershed)
  {
    rv=upSampleAndRecoverEdge(sampled_watershed,src);
    delete sampled_watershed;
  }
  delete sampled;
  delete seed;
  return rv;

}


void ZStackMultiScaleWatershed::fillSeed(ZStack* seed,QList<ZSwcTree*>& trees,const ZIntPoint& offset)
{
  seed->setOffset(offset.m_x,offset.m_y,offset.m_z);
  uint seed_index=1;
  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree* tree=(*it)->clone();
    tree->rescale(1.0/_scale,1.0/_scale,1.0/_scale);
    ZSwcForest* forest=tree->toSwcTreeArray();
    for(std::vector<ZSwcTree*>::iterator x =forest->begin();x!=forest->end();++x)
    {
      (*x)->labelStack(seed,seed_index++);
    }
    std::cout<<"number of seeds area:"<<forest->size()<<std::endl;
    delete tree;
    delete forest;
  }
}
#endif

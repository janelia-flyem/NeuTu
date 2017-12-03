#include <fstream>
#include <QFile>
#include <QTime>
#include <QCoreApplication>
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
#include "widgets/zpythonprocess.h"
#ifdef _DEBUG_
#include "zstackframe.h"
#include "sandbox/zsandbox.h"
#include "mainwindow.h"
#include "zstackdocdatabuffer.h"
#endif
#include "neutubeconfig.h"
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
      offset=z*slice+y*width+2;
      for (int x = 2; x <= width-2; ++x,++offset)
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
          maskArray[offset] = maskArray[offset-1]=maskArray[offset-2]=maskArray[offset+1]=index_map[v][t];
        }
        else
        {
         maskArray[offset] = maskArray[offset-1]=maskArray[offset-2]=maskArray[offset+1]= index_map[v][t]=index++;
        }
      }
    }
  }

  for (int z = 0; z <=depth-1; ++z)//y scan
  {
    for (int x = 0; x <= width-1; ++x)
    {
      offset = slice * z + x + 2*width;
      for (int y = 2; y <= height-2; ++y,offset += width)
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
          maskArray[offset] = maskArray[offset-width]=maskArray[offset-2*width]=maskArray[offset+width]=index_map[v][t];
        }
        else
        {
          maskArray[offset] = maskArray[offset-width]=maskArray[offset-2*width]=maskArray[offset+width]= index_map[v][t]=index++;
        }
      }
    }
  }

  for (int y = 0; y <= height-1; ++y)//z scan
  {
    for (int x = 0; x <=width-1; ++x)
    {
      offset = width * y + x + 2*slice;
      for (int z = 2; z <= depth-2; ++z,offset += slice)
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
          maskArray[offset] = maskArray[offset-slice]=maskArray[offset-2*slice]=maskArray[offset+slice]=index_map[v][t];
        }
        else
        {
          maskArray[offset] = maskArray[offset-slice]=maskArray[offset-2*slice]=maskArray[offset+slice]= index_map[v][t]=index++;
        }
      }
    }
  }
  return mask;
}


void ZStackMultiScaleWatershed::computeSeeds(ZStack* sampled_stack,std::vector<ZObject3d*>& seeds)
{
  int width=sampled_stack->getDsIntv().getX();
  int height=sampled_stack->getDsIntv().getY();
  int depth=sampled_stack->getDsIntv().getZ();
  ZStack* boundary_map=getBoundaryMap(*sampled_stack);
  ZStack* sds=new ZStack(GREY,width,height,depth,1);
  generateSeeds(sds,width,height,depth,boundary_map,sampled_stack);
  seeds.clear();
  seeds=ZObject3dFactory::MakeObject3dArray(*sds);
  delete sds;
  delete boundary_map;
}


//generate seeds for second pass segmentation
void ZStackMultiScaleWatershed::generateSeeds(ZStack* seed,int width,int height,int depth,const ZStack* boundary_map,const ZStack* stack)
{
  int step=m_scale;
  int s_width=stack->width();
  int s_height=stack->height();
  int s_depth=stack->depth();
  size_t s_slice=s_width*s_height;

  const uint8_t* pstack=stack->array8();
  const uint8_t* pboundary=boundary_map->array8();

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
            addSeedX(seed,x*step+step-1,y*step,std::min(height,(y+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
          if(pboundary[_offset+1])//right voxel is on the boundary
          {
            addSeedX(seed,x*step,y*step,std::min(height,(y+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
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
            addSeedY(seed,y*step+step-1,x*step,std::min(width,(x+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
          }
          if(pboundary[_offset+s_width])
          {
            addSeedY(seed,y*step,x*step,std::min(width,(x+1)*step),z*step,std::min(depth,(z+1)*step),pstack[_offset]);
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
            addSeedZ(seed,z*step+step-1,x*step,std::min(width,(x+1)*step),y*step,std::min((y+1)*step,height),pstack[_offset]);
          }
          if(pboundary[_offset+s_slice])
          {
            addSeedZ(seed,z*step,x*step,std::min(width,(x+1)*step),y*step,std::min((y+1)*step,height),pstack[_offset]);
          }
        }
      }
    }
  }
}


//label area needed on second pass segmentation;  and return the bounding box
ZStack* ZStackMultiScaleWatershed::labelAreaNeedUpdate(ZStack* boundary_map,ZStack* seed,ZIntCuboid& boundbox,ZStack* srcStack)
{
  int scale=m_scale;
  int width=seed->width();
  int height=seed->height();
  int depth=seed->depth();
  size_t slice=width*height;

  int s_width=boundary_map->width();
  int s_height=boundary_map->height();
  int s_depth=boundary_map->depth();
  size_t s_slice=s_width*s_height;

  ZStack* rv=new ZStack(GREY,width,height,depth,1);
  uint8_t* pmap=boundary_map->array8();
  uint8_t* pdst=rv->array8();
  uint8_t* psrc=srcStack?srcStack->array8():NULL;


  int min_x=MAX_INT32,min_y=MAX_INT32,min_z=MAX_INT32;
  int max_x=0,max_y=0,max_z=0;
#ifdef _DEBUG_
  size_t cnt=0;
#endif
  for(int z=0;z<s_depth;++z){
    for(int y=0;y<s_height;++y){
      for(int x=0;x<s_width;++x){
        if(pmap[x+y*s_width+z*s_slice]){

          int start_x=std::max(0,x*scale-1),end_x=std::min(width-1,(x+1)*scale);
          int start_y=std::max(0,y*scale-1),end_y=std::min(height-1,(y+1)*scale);
          int start_z=std::max(0,z*scale-1),end_z=std::min(depth-1,(z+1)*scale);

          if(start_x<min_x)min_x=start_x;
          if(end_x>max_x)max_x=end_x;
          if(start_y<min_y)min_y=start_y;
          if(end_y>max_y)max_y=end_y;
          if(start_z<min_z)min_z=start_z;
          if(end_z>max_z)max_z=end_z;

          for(int k=start_z;k<=end_z;++k){
            for(int j=start_y;j<=end_y;++j){
              for(int i=start_x;i<=end_x;++i){
                pdst[i+j*width+k*slice]=srcStack?(psrc[i+j*width+k*slice]):1;
#ifdef _DEBUG_
                ++cnt;
#endif
              }
            }
          }

        }
      }
    }
  }
#ifdef _DEBUG_
  std::cout<<"----------# voxels needed update"<<cnt<<std::endl;
#endif
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
  int width=src->width(),height=src->height();
  size_t slice=width*height;
#ifdef _DEBUG_
  QTime time;
  time.start();
#endif
  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),sampled_watershed);
#ifdef _DEBUG_
  std::cout<<"----------upsample time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  recovered->setOffset(src->getOffset());

#ifdef _DEBUG_
  time.restart();
#endif
  ZStack* boundary_map=getBoundaryMap(*sampled_watershed);
#ifdef _DEBUG_
  std::cout<<"----------boundary map time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);

#ifdef _DEBUG_
  time.restart();
#endif
  generateSeeds(seed,src->width(),src->height(),src->depth(),boundary_map,sampled_watershed);
#ifdef _DEBUG_
  std::cout<<"----------generate seeds time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  ZIntCuboid box;

#ifdef _DEBUG_
  time.restart();
#endif
  ZStack* src_clone=labelAreaNeedUpdate(boundary_map,seed,box,src);
#ifdef _DEBUG_
  std::cout<<"----------label area time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  src_clone->crop(box);
#ifdef _DEBUG_
  std::cout<<"----------# voxels within bounding box:"<<src_clone->getVoxelNumber()<<std::endl;
#endif
  seed->crop(box);

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);

#ifdef _DEBUG_
  time.restart();
#endif
  ZStack* result=watershed.run(src_clone,seed);
#ifdef _DEBUG_
  std::cout<<"----------second pass seg time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  if(!result){
    std::cout<<"local watershed failed"<<std::endl;
    return recovered;
  }

  int s_x=box.getFirstCorner().getX();
  int s_y=box.getFirstCorner().getY();
  int s_z=box.getFirstCorner().getZ();


  uint8_t *pres=result->array8();
  uint8_t *prec=recovered->array8();

#ifdef _DEBUG_
  time.restart();
#endif
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
#ifdef _DEBUG_
  std::cout<<"----------update time:"<<time.elapsed()/1000.0<<std::endl;
#endif

#ifdef _DEBUG_
  std::cout<<"----------# total voxels:"<<recovered->getVoxelNumber()<<std::endl;
#endif
  delete seed;
  delete result;
  delete boundary_map;
  delete src_clone;
  return recovered;

}


#if defined(_QT_GUI_USED_)
ZStack* ZStackMultiScaleWatershed::run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale,const QString &algorithm)
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
#ifdef _DEBUG_
  QTime time;
  time.start();
#endif
  sampled->downsampleMinIgnoreZero(m_scale-1,m_scale-1,m_scale-1);
  //sampled->downsampleMean(m_scale-1,m_scale-1,m_scale-1);
#ifdef _DEBUG_
  std::cout<<"----------downsample time:"<<time.elapsed()/1000.0<<std::endl;
#endif

  seed=toSeedStack(seeds,sampled->width(),sampled->height(),sampled->depth(),sampled->getOffset());

#ifdef _DEBUG_
  time.restart();
#endif
  ZStack* sampled_watershed=NULL;
  if(algorithm=="watershed"){
    sampled_watershed=watershed.run(sampled,seed);
  }
  else if(algorithm=="random_walker"){
    std::string working_dir = NeutubeConfig::getInstance().getPath(NeutubeConfig::WORKING_DIR);
        //on QCoreApplication::applicationDirPath()+"/../python/service/random_walker";
    sampled->setOffset(0,0,0);
    seed->setOffset(0,0,0);
    sampled->save(working_dir+"/data.tif");
    seed->save(working_dir+"/seed.tif");

    ZPythonProcess python;
    python.setWorkDir(working_dir.c_str());
    python.setScript((working_dir+"/random_walker.py").c_str());
    python.addArg((working_dir+"/data.tif").c_str());
    python.addArg((working_dir+"/seed.tif").c_str());
    python.addArg((working_dir+"/result.tif").c_str());

    sampled_watershed=new ZStack();
    python.run();
    sampled_watershed->load(working_dir+"/result.tif");
  }
  else if(algorithm=="power_watershed"){
    const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/power_watershed";
    sampled->setOffset(0,0,0);
    seed->setOffset(0,0,0);
    sampled->save(working_dir.toStdString()+"/data.tif");
    seed->save(working_dir.toStdString()+"/seed.tif");

    ZPythonProcess python;
    python.setWorkDir(working_dir);
    python.setScript(working_dir+"/power_watershed.py");
    python.addArg(working_dir+"/data.tif");
    python.addArg(working_dir+"/seed.tif");
    python.addArg(working_dir+"/result.tif");
    python.addArg("2");
    sampled_watershed=new ZStack();
    python.run();
    sampled_watershed->load(working_dir.toStdString()+"/result.tif");
  }
  else if(algorithm=="MSF_Cruskal"){
    const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/power_watershed";
    sampled->setOffset(0,0,0);
    seed->setOffset(0,0,0);
    sampled->save(working_dir.toStdString()+"/data.tif");
    seed->save(working_dir.toStdString()+"/seed.tif");

    ZPythonProcess python;
    python.setWorkDir(working_dir);
    python.setScript(working_dir+"/power_watershed.py");
    python.addArg(working_dir+"/data.tif");
    python.addArg(working_dir+"/seed.tif");
    python.addArg(working_dir+"/result.tif");
    python.addArg("1");
    sampled_watershed=new ZStack();
    python.run();
    sampled_watershed->load(working_dir.toStdString()+"/result.tif");
  }
  else if(algorithm=="MSF_Prime"){
    const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/power_watershed";
    sampled->setOffset(0,0,0);
    seed->setOffset(0,0,0);
    sampled->save(working_dir.toStdString()+"/data.tif");
    seed->save(working_dir.toStdString()+"/seed.tif");

    ZPythonProcess python;
    python.setWorkDir(working_dir);
    python.setScript(working_dir+"/power_watershed.py");
    python.addArg(working_dir+"/data.tif");
    python.addArg(working_dir+"/seed.tif");
    python.addArg(working_dir+"/result.tif");
    python.addArg("3");
    sampled_watershed=new ZStack();
    python.run();
    sampled_watershed->load(working_dir.toStdString()+"/result.tif");
  }
#ifdef _DEBUG_
  std::cout<<"----------downsample seg time:"<<time.elapsed()/1000.0<<std::endl;
  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(sampled);
  std::vector<ZObject3dScan*> objArray =
      ZObject3dFactory::MakeObject3dScanPointerArray(*sampled_watershed, 1, false);
  ZColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
  int colorIndex = 0;

  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter)
  {
    ZObject3dScan *obj = *iter;

    if (obj != NULL && !obj->isEmpty())
    {
      QColor color = colorScheme.getColor(colorIndex++);
      color.setAlpha(164);
      obj->setColor(color);
      frame->document()->getDataBuffer()->addUpdate(
            obj,ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
      frame->document()->getDataBuffer()->deliver();
    }
  }
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
#endif


  if(sampled_watershed){
    rv=upSampleAndRecoverBoundary(sampled_watershed,src);
#ifdef _DEBUG_
    std::vector<ZObject3d*> sds;
    ZStack* ss=src->clone();
    ss->setOffset(0,0,0);
    sampled_watershed->setDsIntv(src->width(),src->height(),src->depth());
    computeSeeds(sampled_watershed,sds);
    int label=10;
    for(auto x:sds)
    {
      x->labelStack(ss,label);
      label+=20;
    }
    ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(ss);
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);
#else

#endif
    delete sampled_watershed;
  }
#ifndef _DEBUG_
  delete sampled;
#endif
  delete seed;
  return rv;
}


ZStack* ZStackMultiScaleWatershed::toSeedStack(std::vector<ZObject3d*>& seeds,int width,int height,int depth,ZIntPoint offset)
{
  ZStack* mask=new ZStack(GREY,width,height,depth,1);
  mask->setOffset(offset);
  for(ZObject3d* seed:seeds){
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
      if (IS_IN_CLOSE_RANGE(x, 0, width - 1) &&
          IS_IN_CLOSE_RANGE(y, 0, height - 1) &&
          IS_IN_CLOSE_RANGE(z, 0, depth - 1)) {
        array[z * area + y * width + x] = label;
      }
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

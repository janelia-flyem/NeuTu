#include <fstream>
#if defined(_QT_GUI_USED_)
#include <QFile>
#include <QTime>
#include <QCoreApplication>
#endif

#include "geometry/zintcuboid.h"
#include "geometry/zcuboid.h"
#include "geometry/zintcuboid.h"

#include "neutubeconfig.h"

#include "zstackmultiscalewatershed.h"
#include "zstackwatershed.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackfactory.h"
#include "zswcforest.h"
#include "zswctree.h"
#include "zobject3d.h"
#include "zstackdocdatabuffer.h"
#include "zdownsamplefilter.h"

#include "widgets/zpythonprocess.h"
#include "mvc/zstackframe.h"
#include "sandbox/zsandbox.h"
#include "mainwindow.h"

#include "zstackdocdatabuffer.h"
#include "neutubeconfig.h"
#include "zdownsamplefilter.h"
#include "zwatershedmst.h"


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
  unsigned char index_map[256][256] = {{0}};
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
  int width=sampled_stack->getDsIntv().getX()*sampled_stack->width();
  int height=sampled_stack->getDsIntv().getY()*sampled_stack->height();
  int depth=sampled_stack->getDsIntv().getZ()*sampled_stack->depth();
  ZStack* boundary_map=getBoundaryMap(*sampled_stack);
  ZStack* sds=new ZStack(GREY,width,height,depth,1);
  generateSeeds(sds,boundary_map,sampled_stack);
  seeds.clear();
  seeds=ZObject3dFactory::MakeObject3dArray(*sds);
  delete sds;
  delete boundary_map;
}


//generate seeds for second pass segmentation
void ZStackMultiScaleWatershed::generateSeeds(ZStack* seed,const ZStack* boundary_map,const ZStack* stack)
{
  int step=m_scale;
  int s_width=stack->width();
  int s_height=stack->height();
  int s_depth=stack->depth();
  size_t s_slice=s_width*s_height;

  const uint8_t* pstack=stack->array8();
  const uint8_t* pboundary=boundary_map->array8();

  int offset=0,_offset=0;

  for(int z=0;z<s_depth;++z)  //x scan
  {
    offset=z*s_slice;
    for(int y=0;y<s_height;++y)
    {
      _offset=y*s_width+offset;
      for(int x=0;x<s_width;++x,++_offset)
      {
        if(!pboundary[_offset])// current voxel is not on the boundary
        {
          //and all neighbors belong to the same area and at least one neighbor on the boundary
          if(checkNeighbors(pboundary,pstack,x,y,z,s_width,s_height,s_depth))
          {
            addSeed(seed,x*step,x*step+step-1,y*step,y*step+step-1,z*step,z*step+step-1,pstack[_offset]);
          }
        }
      }
    }
  }
}


//check if (x,y,z) satisfies conditions of being a seed point
bool ZStackMultiScaleWatershed::checkNeighbors(const uint8_t* pboundary, const uint8_t* pstack,int x,int y, int z,int width,int height,int depth)
{
  size_t slice=width*height;
  bool has_neighbor_on_boundary=false;
  uint8_t v=0;
  if(x>0){
    size_t off=x-1+y*width+z*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  if(x<width-1){
    size_t off=x+1+y*width+z*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  if(y>0){
    size_t off=x+(y-1)*width+z*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  if(y<height-1){
    size_t off=x+(y+1)*width+z*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  if(z>0){
    size_t off=x+y*width+(z-1)*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  if(z<depth-1){
    size_t off=x+y*width+(z+1)*slice;
    if(v==0){
      v=pstack[off];
    }
    else if(pstack[off]!=v){
      return false;
    }
    if(pboundary[off]){
      has_neighbor_on_boundary=true;
    }
  }
  return has_neighbor_on_boundary;
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

  size_t cnt=0;

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

                ++cnt;

              }
            }
          }

        }
      }
    }
  }
  std::cout<<"----------# voxels needed update"<<cnt<<std::endl;

  boundbox.set(min_x,min_y,min_z,max_x,max_y,max_z);
  return rv;
}

/*
ZStackMultiScaleWatershed::ZStackMultiScaleWatershed()
{

}
*/

ZStackMultiScaleWatershed::~ZStackMultiScaleWatershed()
{

}


ZStack* ZStackMultiScaleWatershed::upSampleAndRecoverBoundary(ZStack* sampled_watershed,ZStack* src)
{
  int width=src->width(),height=src->height();
  size_t slice=width*height;

  QTime time;
  time.start();

  ZStack* recovered=upSample(src->width(),src->height(),src->depth(),sampled_watershed);

  std::cout<<"----------upsample time:"<<time.elapsed()/1000.0<<std::endl;


  recovered->setOffset(src->getOffset());


  time.restart();

  ZStack* boundary_map=getBoundaryMap(*sampled_watershed);

  std::cout<<"----------boundary map time:"<<time.elapsed()/1000.0<<std::endl;


  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);


  time.restart();

  generateSeeds(seed,boundary_map,sampled_watershed);

  std::cout<<"----------generate seeds time:"<<time.elapsed()/1000.0<<std::endl;


  ZIntCuboid box;


  time.restart();

  ZStack* src_clone=labelAreaNeedUpdate(boundary_map,seed,box,src);

  std::cout<<"----------label area time:"<<time.elapsed()/1000.0<<std::endl;


  src_clone->crop(box);

  std::cout<<"----------voxels within bounding box:"<<src_clone->getVoxelNumber()<<std::endl;

  seed->crop(box);

  ZStackWatershed watershed;
  watershed.setFloodingZero(false);


  time.restart();

  ZStack* result=watershed.run(src_clone,seed);

  std::cout<<"----------second pass seg time:"<<time.elapsed()/1000.0<<std::endl;


  if(!result){
    std::cout<<"local watershed failed"<<std::endl;
    return recovered;
  }

  int s_x=box.getFirstCorner().getX();
  int s_y=box.getFirstCorner().getY();
  int s_z=box.getFirstCorner().getZ();


  uint8_t *pres=result->array8();
  uint8_t *prec=recovered->array8();


  time.restart();

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

  std::cout<<"----------update time:"<<time.elapsed()/1000.0<<std::endl;



  std::cout<<"----------# total voxels:"<<recovered->getVoxelNumber()<<std::endl;

  delete seed;
  delete result;
  delete boundary_map;
  delete src_clone;
  return recovered;

}


#if defined(_QT_GUI_USED_)
ZStack* ZStackMultiScaleWatershed::run(ZStack *src,std::vector<ZObject3d*>& seeds,int scale,const QString &algorithm,const QString &dsMethod)
{
  m_scale=scale;
  ZStack* rv=NULL;
  ZStack* seed=NULL;
  ZStackWatershed watershed;

/*  if(m_scale==1){
    seed=toSeedStack(seeds,src->width(),src->height(),src->depth(),src->getOffset());
    seed->setOffset(src->getOffset());
    rv=watershed.run(src,seed);
    delete seed;
    return rv;
  }*/


  //down sample src stack
  ZStack* sampled=NULL;

  QTime time;
  time.start();

  if(m_scale!=1){
    ZDownsampleFilter* downsample=NULL;
    if (dsMethod=="Min"){
      downsample=new ZMinDsFilter();
    }
    else if (dsMethod=="Min(ignore zero)")
    {
      downsample=new ZMinIgnoreZeroDsFilter();
    }
    else if (dsMethod=="Max")
    {
      downsample=new ZMaxDsFilter();
    }
    else if (dsMethod=="Mean")
    {
      downsample=new ZMeanDsFilter();
    }
    else if (dsMethod=="Edge")
    {
      downsample=new ZEdgeDsFilter();
    }
    downsample->setDsFactor(scale,scale,scale);
    sampled=downsample->filterStack(*src);
    delete downsample;
    /*ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(sampled);
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);*/
  }
  else{
    sampled=src->clone();
  }

  std::cout<<"----------downsample time:"<<time.elapsed()/1000.0<<std::endl;

  seed = toSeedStack(seeds,sampled->width(),sampled->height(),sampled->depth(),sampled->getOffset());

  //
  /*if(scale > 1){
    const uint8_t* pSeed = seed->array8();
    const uint8_t* const pSeedEnd = pSeed + seed->getVoxelNumber();
    uint8_t * pSampled = sampled->array8();
    for(; pSeed != pSeedEnd; ++pSeed, ++pSampled){
      if(*pSeed){
        *pSampled = 255;
      }
    }
  }*/

  time.restart();

  ZStack* sampled_watershed=NULL;

  if(algorithm == "" || algorithm=="watershed"){
    sampled_watershed=watershed.run(sampled,seed);
  }

  /*else if (algorithm == "watershedmst")
  {
    ZWatershedMST mst(sampled,seed,m_alpha,m_beta);
    sampled_watershed = mst.run();
  }*/
  else if(algorithm=="random_walker"){
    //std::string working_dir = NeutubeConfig::getInstance().getPath(NeutubeConfig::WORKING_DIR);
        //on QCoreApplication::applicationDirPath()+"/../python/service/random_walker";
    const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/random_walker";
    sampled->setOffset(0,0,0);
    seed->setOffset(0,0,0);
    sampled->save(working_dir.toStdString()+"/data.tif");
    seed->save(working_dir.toStdString()+"/seed.tif");

    ZPythonProcess python;
    python.setWorkDir(working_dir);
    python.setScript(working_dir+"/random_walker.py");
    python.addArg(working_dir+"/data.tif");
    python.addArg(working_dir+"/seed.tif");
    python.addArg(working_dir+"/result.tif");

    sampled_watershed=new ZStack();
    python.run();
    sampled_watershed->load(working_dir.toStdString()+"/result.tif");
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
  else if(algorithm=="FFN"){
    const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/ffn";
    //sampled->setOffset(0,0,0);
    //seed->setOffset(0,0,0);
    sampled->save(working_dir.toStdString()+"/data.tif");
    //seed->save(working_dir.toStdString()+"/seed.tif");
    std::ofstream out(working_dir.toStdString()+"/seed.txt");
    for(int k=0;k<seed->depth();++k){
      for(int j=0;j<seed->height();++j){
        for(int i=0;i<seed->width();++i){
          if(seed->array8()[i+j*seed->width()+k*seed->width()*seed->height()]){
            out<<k<<" "<<j<<" "<<i<<std::endl;
          }
        }
      }
    }
    ZPythonProcess python;
    python.setWorkDir(working_dir);
    python.setScript(working_dir+"/ffn_skeleton.py");
    python.addArg(working_dir+"/data.tif");
    python.addArg(working_dir+"/result.tif");
    python.addArg(working_dir+"/seed.txt");
    sampled_watershed=new ZStack();
    python.run();
//    ZStack* middle_result=new ZStack();

    sampled_watershed->load(working_dir.toStdString()+"/result.tif");

    //sampled_watershed=watershed.run(sampled,middle_result);
    //delete middle_result;
  }
  std::cout<<"----------downsample seg time:"<<time.elapsed()/1000.0<<std::endl;

#if 0  //show sampled stack segmentation result
  {
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
              obj,ZStackDocObjectUpdate::EAction::ACTION_ADD_UNIQUE);
        frame->document()->getDataBuffer()->deliver();
      }
    }
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);
  }
#endif


  if(sampled_watershed){
    if(m_scale!=1){
      rv=upSampleAndRecoverBoundary(sampled_watershed,src);
    }
    else{
      rv=sampled_watershed->clone();
    }

#if 0 //show seeds
    {
      std::vector<ZObject3d*> sds;
      ZStack* ss=src->clone();
      ss->setOffset(0,0,0);
      sampled_watershed->setDsIntv(m_scale,m_scale,m_scale);
      computeSeeds(sampled_watershed,sds);
      ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(ss);
      ZColorScheme colorScheme;
      colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
      int colorIndex = 0;

      for (std::vector<ZObject3d*>::iterator iter = sds.begin();
           iter != sds.end(); ++iter)
      {
        ZObject3d *obj = *iter;
        QColor color = colorScheme.getColor(colorIndex++);
        color.setAlpha(164);
        obj->setColor(color);
        frame->document()->getDataBuffer()->addUpdate(
                obj,ZStackDocObjectUpdate::EAction::ACTION_ADD_UNIQUE);
        frame->document()->getDataBuffer()->deliver();

      }
      ZSandbox::GetMainWindow()->addStackFrame(frame);
      ZSandbox::GetMainWindow()->presentStackFrame(frame);
    }
#endif
    delete sampled_watershed;
  }

  //delete sampled;

  delete seed;
  rv->setOffset(src->getOffset());
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



void ZStackMultiScaleWatershed::addSeed(ZStack* pSeed,int sx,int ex,int sy,int ey,int sz,int ez,uint8_t v){
  uint8_t* p=pSeed->array8();
  int width=pSeed->width();
  int height=pSeed->height();
  int depth=pSeed->depth();
  size_t slice=height*width;

  for(int z=sz;z<std::min(depth,ez);++z){
    for(int y=sy;y<std::min(height,ey);++y){
      for(int x=sx;x<std::min(width,ex);++x){
        if (IS_IN_CLOSE_RANGE(x, 0, width - 1) &&
            IS_IN_CLOSE_RANGE(y, 0, height - 1) &&
            IS_IN_CLOSE_RANGE(z, 0, depth - 1)) {
          p[x+width*y+slice*z]=v;
        }
      }
    }
  }
}

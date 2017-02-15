#include <QAction>
#include <QWidget>
#include <QSpinBox>
#include <QMessageBox>
#include <vector>
#include <QPushButton>
#include <QGridLayout>
#include <cstdlib>
#include "zmultiscalewatershedmodule.h"
#include "imgproc/zstackwatershed.h"
#include "zstackdoc.h"
#include "zsandbox.h"
#include "mainwindow.h"
#include "neutubeconfig.h"
#include "zobject3dscan.hpp"
//#include "zdownsamplingmodule.h"

ZMultiscaleWaterShedModule::ZMultiscaleWaterShedModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


ZMultiscaleWaterShedModule::~ZMultiscaleWaterShedModule()
{
  delete window;
}


void ZMultiscaleWaterShedModule::init()
{  
  m_action = new QAction("Multiscale WaterShed", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
  window=new ZWaterShedWindow();
}


void ZMultiscaleWaterShedModule::execute()
{
  window->show();
}


void getEdgePoints(ZStack* stack,std::vector<ZIntPoint>** metrix)
{
  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();
  int slice=width*height;
  uint8_t* po=stack->array8();
  for(int k=0;k<depth;++k)
  {
    size_t offset=k*slice;
    for(int j=0;j<height;++j)
    {
      size_t _offset=offset+j*width;
      for(int i=0;i<width;++i)
      {
        uint8_t v=po[_offset+i];
        for(int z=std::max(0,k-1);z<=std::min(depth-1,k+1);++z)
          for(int y=std::max(0,j-1);y<=std::min(height-1,j+1);++y)
            for(int x=std::max(0,i-1);x<=std::min(width-1,i+1);++x)
            {
              uint8_t t=po[z*slice+y*width+x];
              if(v<t)
              {
                metrix[v-1][t-1].push_back(ZIntPoint(i,j,k));
                metrix[t-1][v-1].push_back(ZIntPoint(x,y,z));
              }
            }
      }
    }
  }

}

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


ZStack* getBoundBoxes(ZStack* stack,std::vector<ZIntCuboid>&boxes)
{
  int width=stack->width();
  int height=stack->height();
  int slice=width*height;

  int size=std::ceil(stack->max());
  std::vector<ZIntPoint>** metrix=new std::vector<ZIntPoint>*[size];
  for(int i=0;i<size;++i)
  {
    metrix[i]=new std::vector<ZIntPoint>[size];
  }
  getEdgePoints(stack,metrix);

  ZStack* rv=new ZStack(stack->kind(),width,height,stack->depth(),stack->channelNumber());
  uint8_t* pr=rv->array8();
  rv->setZero();
  int index=1;
  for(int i=0;i<size-1;++i)
  {
    for(int j=i+1;j<size;++j)
    {
      int s=metrix[i][j].size();
      for(int k=0;k<s;++k)
      {
        const ZIntPoint& p=metrix[i][j][k];
        const ZIntPoint& q=metrix[j][i][k];
        pr[p.m_x+p.m_y*width+p.m_z*slice]=index;
        pr[q.m_x+q.m_y*width+q.m_z*slice]=index;
      }
      if(s)
      {
        index++;
        assert(index<256);
      }
    }
  }

  for(int i=0;i<size;++i)
  {
    delete[] metrix[i];
  }
  delete[] metrix;

  std::vector<ZObject3dScan*>edges=ZObject3dScan::extractAllObject(*rv);
  for(std::vector<ZObject3dScan*>::iterator it=edges.begin();it!=edges.end();++it)
  {
    boxes.push_back((*it)->getBoundBox());
    delete *it;
  }
  return rv;
}


inline bool isEdgePoint(const ZStack* edge_map,int x,int y,int z)
{
  int width=edge_map->width();
  int height=edge_map->height();
  int slice=width*height;
  const uint8_t* p=edge_map->array8();
  return p[x+y*width+z*slice];
}

inline bool hasNeighborOnEdge(const ZStack* edge_map,int x,int y,int z)
{
  int width=edge_map->width();
  int height=edge_map->height();
  int depth=edge_map->depth();
  int slice=width*height;
  const uint8_t* p=edge_map->array8();
  if(x>0 && p[x-1+y*width+z*slice])
  {
    return true;
  }
  if(x<width-1 && p[x+1+y*width+z*slice])
  {
    return true;
  }
  if(y>0 && p[x+(y-1)*width+z*slice])
  {
    return true;
  }
  if(y<height-1 && p[x+(y+1)*width+z*slice])
  {
    return true;
  }
  if(z>0 && p[x+y*width+(z-1)*slice])
  {
    return true;
  }
  if(z<depth-1 && p[x+y*width+(z+1)*slice])
  {
    return true;
  }
  return false;
}

void generateSeedsInSampledSpace(std::vector<ZStack*>& seeds,
                                 const ZIntCuboid& box,
                                 const ZStack* edge_map,
                                 const ZStack* stack,int step)
{
  int width=stack->width();
  int height=stack->height();
  int slice=width*height;
  const uint8_t* ps=stack->array8();

  int start_x=box.getFirstCorner().m_x;
  int start_y=box.getFirstCorner().m_y;
  int start_z=box.getFirstCorner().m_z;
  int end_x=box.getLastCorner().m_x;
  int end_y=box.getLastCorner().m_y;
  int end_z=box.getLastCorner().m_z;

  //find points which are appropriate to be seeds in the box
  for(int z=start_z;z<=end_z;++z)
  {
    for(int y=start_y;y<=end_y;++y)
    {
      for(int x=start_x;x<=end_x;++x)
      {
        if( (!isEdgePoint(edge_map,x,y,z)) && hasNeighborOnEdge(edge_map,x,y,z))
        {
          ZStack* seed=new ZStack(GREY,step,step,step,1);
          int v=ps[x+y*width+z*slice];
          for(uint i=0;i<seed->getVoxelNumber();++i)
          {
            seed->array8()[i]=v;
          }
          seed->setOffset(x,y,z);
          seeds.push_back(seed);
        }
      }
    }
  }
}


void mapSeeds2OriginalSpace(std::vector<ZStack*>& seeds,const int step,int width,int height,int depth)
{
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    ZStack* seed=*it;
    ZIntPoint offset=seed->getOffset();
    int x=offset.m_x*step;
    int y=offset.m_y*step;
    int z=offset.m_z*step;
    if(x>=width)x=width-1;
    if(y>=height)y=height-1;
    if(z>=depth)z=depth-1;
    seed->setOffset(x,y,z);
  }
}


void drawSeeds(const std::vector<ZStack*>& seeds,ZStack* stack,int step,const int v)
{
  uint8_t* p=stack->array8();
  const int width=stack->width();
  const int height=stack->height();
  const int area=width*stack->height();
  for(std::vector<ZStack*>::const_iterator it=seeds.begin();it!=seeds.end();++it)
  {
    const ZStack* seed=*it;
    const ZIntPoint point=seed->getOffset();
    const int x=point.m_x,y=point.m_y,z=point.m_z;
    for(int i=x;i<=std::min(width-1,x+step);++i)
    {
      for(int j=y;j<=std::min(height-1,y+step);++j)
      {
         p[z*area+j*width+i]=v;
       }
    }
  }
}


void drawRange(ZStack* stack,const Cuboid_I& range,int v)
{
  const int x1=range.cb[0],x2=range.ce[0];
  const int y1=range.cb[1],y2=range.ce[1];
  const int z1=range.cb[2],z2=range.ce[2];
  const int width=stack->width();
  const int area=stack->height()*width;
  uint8_t* p=stack->array8();

  for(int k=z1;k<=z2;++k)
    for(int i=x1;i<=x2;++i)
      p[i+y1*width+k*area]=p[i+y2*width+k*area]=v;

  for(int k=z1;k<=z2;++k)
    for(int j=y1;j<=y2;++j)
      p[x1+j*width+k*area]=p[x2+j*width+k*area]=v;

}


void setNoneEdgePoints2Zero(ZStack* stack,const ZStack* edge_map,
                            const Cuboid_I& range,int step)
{

  int width=stack->width(),height=stack->height(),depth=stack->depth(),slice=width*height;

  uint8_t* q=stack->array8();

  for(int z=range.cb[2]/step;z<=range.ce[2]/step;++z)
  {
    for(int y=range.cb[1]/step;y<=range.ce[1]/step;++y)
    {
      for(int x=range.cb[0]/step;x<=range.ce[0]/step;++x)
      {
        if(!isEdgePoint(edge_map,x,y,z) && (!hasNeighborOnEdge(edge_map,x,y,z)))
        {
          for(int d=z*step;d<std::min((z+1)*step,depth);++d)
            for(int h=y*step;h<std::min((y+1)*step,height);++h)
              for(int w=x*step;w<std::min((x+1)*step,width);++w)
                q[d*slice+h*width+w]=0;
        }
      }
    }
  }
}

void localWaterShed(const std::vector<ZStack*>& seeds,
                    const Cuboid_I& range,
                    ZStack* recovered,ZStack* original,const ZStack* edge_map,int step)
{
  ZStackWatershed watershed;
  watershed.setRange(range);
  //set points that are (not seeds and not at any edge) to zero
  setNoneEdgePoints2Zero(original,edge_map,range,step);
  watershed.setFloodingZero(false);
  ZStack* result=watershed.run(original,seeds);


  //merge result into recovered image
  uint8_t* p=recovered->array8();
  uint8_t* q=result->array8();
  int width=original->width();
  int slice=width*original->height();
  int x1=range.cb[0],x2=range.ce[0];
  int y1=range.cb[1],y2=range.ce[1];
  int z1=range.cb[2],z2=range.ce[2];
  int ox=result->getOffset().m_x,oy=result->getOffset().m_y,oz=result->getOffset().m_z;
  int r_w=result->width(),r_s=result->height()*r_w;
  for(int z=z1;z<=z2;++z)
  {
    for(int y=y1;y<=y2;++y)
    {
      for(int x=x1;x<=x2;++x)
      {
        int v=q[x-ox+(y-oy)*r_w+(z-oz)*r_s];
        if(v)
        {
          p[x+y*width+z*slice]=v;
        }
      }
    }
  }
 // recovered->setBlockValue(range.cb[0],range.cb[1],range.cb[2],result);
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
  range.ce[0]=std::min((q.m_x+2)*step,width-1);
  range.ce[1]=std::min((q.m_y+2)*step,height-1);
  range.ce[2]=std::min((q.m_z+2)*step,depth-1);
  return range;
}


void getSeeds(const ZStackDoc * doc,std::vector<ZStack*>& seeds)
{
  ZStack    *signal,*seed;
  if((!doc) || !(signal=doc->getStack()))
  {
    return;
  }
  QList<ZSwcTree*> trees=doc->getSwcList();
  if(trees.size()<2 || trees.size()>255)
  {
    return ;
  }
  uint seed_index=1;
  /*for each swc tree we create a relative seed stack*/
  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree* tree=*it;
    ZCuboid box=tree->getBoundBox();
    seed=new ZStack(signal->kind(),box.width(),box.height(),box.depth(),signal->channelNumber());
    ZPoint _off=box.firstCorner();
    #define ti(x) (static_cast<int>(x))
    ZIntPoint off(ti(_off.getX()),ti(_off.getY()),ti(_off.getZ()));
    #undef ti
    seed->setOffset(off);
    tree->labelStack(seed,seed_index++);
    seeds.push_back(seed);
  }
}


ZStack* WaterShed(const std::vector<ZStack*>& seeds,const ZStack* signal)
{

  ZStackWatershed watershed;
  /*set range of area involved in calculation
   *and run the watershed algorithm*/

  /*Cuboid_I range;
  Cuboid_I_Set_S(&range,0,0,125,1024,1024,1);
  watershed.setRange(range);*/
  return watershed.run(signal,seeds);

}


ZWaterShedWindow::ZWaterShedWindow(QWidget *parent) :
  QWidget(parent)
{
  this->setWindowTitle("Multiscale Watershed");
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  this->setWindowFlags(flags);
  QGridLayout* lay=new QGridLayout(this);
  lay->addWidget(new QLabel("Downsample Scale"),0,0,1,2);
  spin_step=new QSpinBox();
  spin_step->setMinimum(1);
  lay->addWidget(spin_step,0,2);
  ok=new QPushButton("OK");
  cancel=new QPushButton("Cancel");
  lay->addWidget(cancel,1,2);
  lay->addWidget(ok,1,3);
  this->setLayout(lay);
  this->move(300,200);
  connect(ok,SIGNAL(clicked()),this,SLOT(onOk()));
  connect(cancel,SIGNAL(clicked()),this,SLOT(onCancel()));
}


void ZWaterShedWindow::onOk()
{

  int scale=spin_step->value();
  if(scale==1)
  {
    onScaleEqualsOne();
  }
  else
  {
    onScaleLargerThanOne();
  }
}


void ZWaterShedWindow::onCancel()
{
  this->hide();
}


void ZWaterShedWindow::onScaleEqualsOne()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  ZStack    *signal,*result;
  if((!doc) || !(signal=doc->getStack()))
  {
    return;
  }

  std::vector<ZStack*> seeds;
  getSeeds(doc,seeds);

  result=WaterShed(seeds,signal);
  /*create a frame of result to show in the main window*/
  if(result)
  {
#ifdef _DEBUG_
    result->save(GET_TEST_DATA_DIR + "/test.tif");
#endif
    ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(result);
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);
  }

  /*and finally free the memory*/
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    delete *it;
  }
}


void ZWaterShedWindow::onScaleLargerThanOne()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *original=doc->getStack();
  if(!original)return;

  int step=spin_step->value();
  ZStack* sampled=original->clone();
  sampled->downsampleMin(step-1,step-1,step-1);

  std::vector<ZStack*> seeds;

  QList<ZSwcTree*> trees=doc->getSwcList();
  if(trees.size()<2 || trees.size()>255)
  {
    return ;
  }
  uint seed_index=1;
  /*for each swc tree we create a relative seed stack*/
  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree* tree=*it;
    ZCuboid box=tree->getBoundBox();
    ZStack* seed=new ZStack(original->kind(),
                            std::max(1,(int)(box.width()/step)),
                            std::max(1,(int)(box.height()/step)),
                            std::max(1,(int)(box.depth()/step)),
                            original->channelNumber());
    ZPoint _off=box.firstCorner();
    ZIntPoint off(_off.getX()/step,_off.getY()/step,_off.getZ()/step);
    seed->setOffset(off);
    for(int i=0;i<seed->getVoxelNumber();++i)
    {
      seed->array8()[i]=seed_index;
    }
    seed_index++;
    //tree->labelStack(seed,seed_index++);
    seeds.push_back(seed);
  }
/*  getSeeds(doc,seeds);
  for(uint i=0;i<seeds.size();++i)
  {
    seeds[i]->
    seeds[i]->downsampleMin(step-1,step-1,step-1);
  }*/

  ZStack* sampled_water_shed=WaterShed(seeds,sampled);
  if(sampled_water_shed)
  {
    ZStack* o=original->clone();
    uint8_t* p=o->array8();
    for(uint i=0;i<o->getVoxelNumber();++i)
    {
      if(p[i]==0)
        p[i]=1;
    }
    onRecover(sampled_water_shed,o);
    delete o;
    delete sampled_water_shed;
  }
  delete sampled;
  for(std::vector<ZStack*>::iterator it=seeds.begin();it!=seeds.end();++it)
  {
    delete *it;
  }
}


void ZWaterShedWindow::onRecover(ZStack* sampled,ZStack* original)
{

  int width=original->width(),height=original->height(),depth=original->depth();
  int step=spin_step->value();
  ZStack* recovered=recover2OriginalSize(original,sampled,step);

  std::vector<ZIntCuboid>boxes;
  std::vector<ZStack*> seeds;
  //get bound boxes of each each
  ZStack* edge_map=getBoundBoxes(sampled,boxes);

  //process each bound box
  for(uint i=0;i<boxes.size();++i)//deal with each edge
  {
      const ZIntCuboid& box=boxes[i];
      Cuboid_I range=getRange(box,original,step);

      generateSeedsInSampledSpace(seeds,box,edge_map,sampled,step);

      mapSeeds2OriginalSpace(seeds,step,width,height,depth);

      localWaterShed(seeds,range,recovered,original,edge_map,step);

    //  drawSeeds(seeds,zo,step,0);
     // drawRange(zo,range,0);

      for(uint j=0;j<seeds.size();++j)
      {
         delete seeds[j];
      }
      seeds.clear();
  }
  delete edge_map;

  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(recovered);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
 // frame=ZSandbox::GetMainWindow()->createStackFrame(zo);
 // ZSandbox::GetMainWindow()->addStackFrame(frame);
 // ZSandbox::GetMainWindow()->presentStackFrame(frame);
}

#include <QAction>
#include <QWidget>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>
#include <cstdlib>
#include "zsandbox.h"
#include "zstackdoc.h"
#include "mainwindow.h"
#include "zobject3dscan.hpp"
#include "zdownsamplingmodule.h"
#include "zstackwatershed.h"


void getEdges(ZStack* plane,std::vector<ZIntPoint>** metrix)
{
  int width=plane->width();
  int height=plane->height();
  uint8_t* po=plane->array8();
  for(int j=0;j<height;++j)
  {
    size_t offset=j*width;
    for(int i=0;i<width;++i)
    {
      uint8_t v=po[offset+i];
      uint8_t t=i>0?po[offset+i-1]:v;
      if(v<t)metrix[v-1][t-1].push_back(ZIntPoint(i,j,0));
      t=i<(width-1)?po[offset+i+1]:v;
      if(v<t)metrix[v-1][t-1].push_back(ZIntPoint(i,j,0));
      t=j>0?po[offset+i-width]:v;
      if(v<t)metrix[v-1][t-1].push_back(ZIntPoint(i,j,0));
      t=j<(height-1)?po[offset+i+width]:v;
      if(v<t)metrix[v-1][t-1].push_back(ZIntPoint(i,j,0));
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


ZStack* getBoundBoxes(ZStack* plane,std::vector<ZIntCuboid>&boxes,
                   std::vector<int>&vvalue,std::vector<int>&tvalue,int depth)
{

  int width=plane->width();
  int size=std::ceil(plane->max());
  std::vector<ZIntPoint>** metrix=new std::vector<ZIntPoint>*[size];
  for(int i=0;i<size;++i)
  {
    metrix[i]=new std::vector<ZIntPoint>[size];
  }
  getEdges(plane,metrix);
  ZStack* rv=new ZStack(GREY,plane->width(),plane->height(),1,1);
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
        ZIntPoint& p=metrix[i][j][k];
        pr[p.m_x+p.m_y*width]=index;
      }
      if(s)
      {
        vvalue.push_back(i+1);
        tvalue.push_back(j+1);
        index++;
      }
    }
  }
  //free matrix
  for(int i=0;i<size;++i)
  {
    delete[] metrix[i];
  }
  delete[] metrix;

  std::vector<ZObject3dScan*>edges=ZObject3dScan::extractAllObject(*rv);
  for(uint i=0;i<edges.size();++i)
  {
    ZIntCuboid box=edges[i]->getBoundBox();
    box.setFirstZ(depth);
    box.setLastZ(depth);
    boxes.push_back(box);
    delete edges[i];
  }
  return rv;
}


inline int distance2Edge(const ZStack* pEdge,int x,int y)
{
  static int max_dis=10;
  const uint8_t* p=pEdge->array8();
  int width=pEdge->width();
  int height=pEdge->height();

  for(int dis=0;dis<max_dis;++dis)
  {
    for(int j=std::max(y-dis,0);j<=std::min(y+dis,height-1);++j)
    {
      if((x-dis>0) && p[j*width+x-dis])
        return dis;
      if((x+dis<width-1) && p[j*width+x+dis])
        return dis;
    }
    for(int i=std::max(x-dis,0);i<=std::min(x+dis,width-1);++i)
    {
      if((y-dis>0) && p[(y-dis)*width+i])
        return dis;
      if((y+dis<height-1) && p[(y+dis)*width+i])
        return dis;
    }
  }

  return max_dis;
}


void generateSeedsInSampledSpace(std::vector<ZStack*>& seeds,
                                 const ZIntCuboid& box,
                                 const ZStack* edge_map,
                                 const ZStack* plane,
                                 int v,int t)
{
  const int width=plane->width();
  const uint8_t* ps=plane->array8();
  const ZIntPoint&p=box.getFirstCorner();
  const int cur_depth=p.m_z;
  const int start_x=p.m_x;
  const int start_y=p.m_y;
  const int x_width=box.getWidth();
  const int y_width=box.getHeight();
  const int max_times=box.getVolume()/5;
  const int cnt_seed_wanted=box.getVolume()>50?box.getVolume()/50:1;
  const int vec_num=10;

  int cnt_seedx=0;
  int cnt_seedy=0;

  std::vector<ZIntPoint> vec_x_backup[vec_num],vec_y_backup[vec_num];

  //generate some random points and compute their distances to rough edge
  for(int times=0;times<max_times;++times)
  {
    int x=start_x+rand()%x_width;
    int y=start_y+rand()%y_width;
    int value=ps[x+y*width];
    if(v==value)
    {
      int distance=distance2Edge(edge_map,x,y);
      vec_x_backup[distance>(vec_num-1)?(vec_num-1):distance].push_back(ZIntPoint(x,y,cur_depth));
    }
    else if(t==value)
    {
      int distance=distance2Edge(edge_map,x,y);
      vec_y_backup[distance>(vec_num-1)?(vec_num-1):distance].push_back(ZIntPoint(x,y,cur_depth));
    }
  }

  //get seeds according to their distance to rough edge
  for(int i=vec_num-1;i>=0;--i)
  {
    const std::vector<ZIntPoint>&vec=vec_x_backup[i];
    for(uint j=0;cnt_seedx<cnt_seed_wanted && j<vec.size();++j)
    {
      cnt_seedx++;
      const ZIntPoint& p=vec[j];
      ZStack* seed=new ZStack(GREY,1,1,1,1);
      seed->array8()[0]=v;
      seed->setOffset(p.m_x,p.m_y,p.m_z);
      seeds.push_back(seed);
    }
  }
  for(int i=vec_num-1;i>=0;--i)
  {
    const std::vector<ZIntPoint>&vec=vec_y_backup[i];
    for(uint j=0;cnt_seedy<cnt_seed_wanted && j<vec.size();++j)
    {
      cnt_seedy++;
      const ZIntPoint& p=vec[j];
      ZStack* seed=new ZStack(GREY,1,1,1,1);
      seed->array8()[0]=t;
      seed->setOffset(p.m_x,p.m_y,p.m_z);
      seeds.push_back(seed);
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


void drawSeeds(const std::vector<ZStack*>& seeds,ZStack* stack,const int v)
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
    for(int i=std::max(0,x-1);i<std::min(width,x+1);++i)
    {
      for(int j=std::max(0,y-1);j<std::min(height,y+1);++j)
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

 /* for(int j=y1;j<=y2;++j)
    for(int i=x1;i<=x2;++i)
      p[i+j*width+z1*area]=p[i+j*width+z2*area]=v;*/
}


void localWaterShed(const std::vector<ZStack*>& seeds,
                    const Cuboid_I& range,
                    ZStack* recovered,ZStack* original)
{

  ZStackWatershed watershed;
  watershed.setRange(range);
  ZStack* result=watershed.run(original,seeds);
  recovered->setBlockValue(range.cb[0],range.cb[1],range.cb[2],result);
  delete result;
}


Cuboid_I getRange(const ZIntCuboid& box,const ZStack* stack,int step)
{
  Cuboid_I range;
  const ZIntPoint&p=box.getFirstCorner();

  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();

  int box_start_x=p.m_x*step-1;
  if(box_start_x<0)box_start_x=0;
  else if(box_start_x>=width)box_start_x=width-1;

  int box_start_y=(p.m_y*step-1);
  if(box_start_y<0)box_start_y=0;
  else if(box_start_y>=height)box_start_y=height-1;

  int box_start_z=p.m_z*step;
  if(box_start_z>=depth)box_start_z=depth-1;

  int box_w=(box_start_x+(box.getWidth()+2)*step)<width?
        (box.getWidth()+2)*step:width-box_start_x;

  int box_h=(box_start_y+(box.getHeight()+2)*step)<height?
        (box.getHeight()+2)*step:height-box_start_y;

  int box_d=(box_start_z+(box.getDepth())*step)<depth?
        (box.getDepth())*step:depth-box_start_z;

  Cuboid_I_Set_S(&range,box_start_x,box_start_y,box_start_z,box_w,box_h,box_d);

  return range;
}


void ZDownSamplingWindow::onRecover()
{
  if(!original)return;
  ZStack* zo=original->clone();

  int width=original->width(),height=original->height(),depth=original->depth();
  ZStack* sampled=ZSandbox::GetCurrentDoc()->getStack();
  ZStack* recovered=recover2OriginalSize(original,sampled,step);

  std::vector<ZIntCuboid>boxes;
  std::vector<int>vvalue;
  std::vector<int>tvalue;
  std::vector<ZStack*> seeds;

  std::srand(0);
  /*process each plane*/
  for(int d=0;d<sampled->depth();++d)
  {
    ZStack* plane=sampled->makeCrop(ZIntCuboid(0,0,d,sampled->width()-1,sampled->height()-1,d));
    ZStack* edge=getBoundBoxes(plane,boxes,vvalue,tvalue,d);
    for(uint i=0;i<boxes.size();++i)//deal with each edge
    {
      const ZIntCuboid& box=boxes[i];
      Cuboid_I range=getRange(box,original,step);

      generateSeedsInSampledSpace(seeds,box,edge,plane,vvalue[i],tvalue[i]);
      mapSeeds2OriginalSpace(seeds,step,width,height,depth);
      localWaterShed(seeds,range,recovered,original);

      drawSeeds(seeds,zo,0);
      drawRange(zo,range,i);

      for(uint j=0;j<seeds.size();++j)
      {
         delete seeds[j];
      }
      seeds.clear();
    }
    delete edge;
    delete plane;
    boxes.clear();
  }
  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(recovered);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
  frame=ZSandbox::GetMainWindow()->createStackFrame(zo);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
}


ZDownSamplingModule::ZDownSamplingModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


void ZDownSamplingModule::init()
{
  m_action = new QAction("Downsampling", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
  window=new ZDownSamplingWindow();
}


ZDownSamplingModule::~ZDownSamplingModule()
{
  delete window;
}


void ZDownSamplingModule::execute()
{
  window->show();
}


ZDownSamplingWindow::ZDownSamplingWindow(QWidget* parent)
  :QWidget(parent)
{
  this->setWindowTitle("DownSampling");
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  this->setWindowFlags(flags);
  QVBoxLayout* lay=new QVBoxLayout(this);
  down=new QPushButton("downSampling");
  recover=new QPushButton("recover");
  spin_step=new QSpinBox();
  spin_step->setMinimum(2);
  lay->addWidget(down);
  lay->addWidget(recover);
  lay->addWidget(spin_step);
  this->setLayout(lay);
  this->move(300,200);
  connect(down,SIGNAL(clicked()),this,SLOT(onDownSampling()));
  connect(recover,SIGNAL(clicked()),this,SLOT(onRecover()));
  original=NULL;
}


void ZDownSamplingWindow::onDownSampling()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *original_img=doc->getStack();
  if(!original_img)return;

  original=original_img;
  step=spin_step->value();

  ZStack* s=original_img->clone();
  s->downsampleMin(step-1,step-1,step-1);

  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(s);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
}

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


ZStack* recover2OriginalSize(const ZStack* original,ZStack* sampled,int step)
{
  int width=original->width();
  int height=original->height();
  int depth=original->depth();
  int slice=width*height;
  int s_width=sampled->width();
  int s_height=sampled->height();
  int s_depth=sampled->depth();
  int s_slice=s_width*sampled->height();

  ZStack* recover=new ZStack(original->kind(),width,height,depth,original->channelNumber());
  uint8_t* pr=recover->array8();
  uint8_t* ps=sampled->array8();

  size_t s_offset=0;
  for(int k=0;k<s_depth;++k)
  {
    s_offset=k*s_slice;
    for(int j=0;j<s_height;++j)
    {
      size_t ss_offset=s_offset+j*s_width;
      for(int i=0;i<s_width;++i)
      {
        uint8_t v=ps[ss_offset+i];
        for(int d=k*step;d<std::min((k+1)*step,depth);++d)
          for(int h=j*step;h<std::min((j+1)*step,height);++h)
            for(int w=i*step;w<std::min((i+1)*step,width);++w)
              pr[d*slice+h*width+w]=v;
      }
    }
  }
  return recover;
}


void getBoundBoxes(ZStack* plane,std::vector<ZIntCuboid>&boxes,
                   std::vector<int>&vvalue,std::vector<int>&tvalue,
                    std::vector<int>&type,int cur_depth)
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
      if(v<t)
      {
        boxes.push_back(ZIntCuboid(i-1,j,cur_depth,i,j,cur_depth));
        vvalue.push_back(v);
        tvalue.push_back(t);
        type.push_back(1);
        continue;
      }
      t=i<(width-1)?po[offset+i+1]:v;
      if(v<t)
      {
        boxes.push_back(ZIntCuboid(i,j,cur_depth,i+1,j,cur_depth));
        vvalue.push_back(v);
        tvalue.push_back(t);
        type.push_back(2);
        continue;
      }
      t=j>0?po[offset+i-width]:v;
      if(v<t)
      {
        boxes.push_back(ZIntCuboid(i,j-1,cur_depth,i,j,cur_depth));
        vvalue.push_back(v);
        tvalue.push_back(t);
        type.push_back(3);
        continue;
      }
      t=j<(height-1)?po[offset+i+width]:v;
      if(v<t)
      {
        boxes.push_back(ZIntCuboid(i,j,cur_depth,i,j+1,cur_depth));
        vvalue.push_back(v);
        tvalue.push_back(t);
        type.push_back(4);
      }
    }
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


Cuboid_I mapBox2OriginalSpace(const ZIntCuboid& box,const ZStack* stack,int step)
{
  Cuboid_I range;
  const ZIntPoint&p=box.getFirstCorner();

  int width=stack->width();
  int height=stack->height();
  int depth=stack->depth();

  int box_start_x=p.m_x*step-1;
  if(box_start_x<0)box_start_x=0;
  else if(box_start_x>=width)box_start_x=width-1;

  int box_start_y=p.m_y*step-1;
  if(box_start_y<0)box_start_y=0;
  else if(box_start_y>=height)box_start_y=height-1;

  int box_start_z=p.m_z*step;
  if(box_start_z>=depth)box_start_z=depth-1;

  int box_w=(box_start_x+box.getWidth()*step+2)<width?
        box.getWidth()*step+2:width-box_start_x;

  int box_h=(box_start_y+box.getHeight()*step+2)<height?
        box.getHeight()*step+2:height-box_start_y;

  int box_d=(box_start_z+(box.getDepth())*step)<depth?
        box.getDepth()*step:depth-box_start_z;

  Cuboid_I_Set_S(&range,box_start_x,box_start_y,box_start_z,box_w,box_h,box_d);

  return range;
}

void generateSeedsInOriginalSpace(std::vector<ZStack*>&seeds,const Cuboid_I& range,
                                  int v,int t,int type)
{
  ZStack* seedv=new ZStack(GREY,1,1,1,1);
  seedv->array8()[0]=v;
  seeds.push_back(seedv);
  ZStack* seedt=new ZStack(GREY,1,1,1,1);
  seedt->array8()[0]=t;
  seeds.push_back(seedt);

  int left_x=range.cb[0];
  int right_x=range.ce[0];
  int left_y=range.cb[1];
  int right_y=range.ce[1];
  int z=range.cb[2];

  if(type==1)
  {
    int y=(left_y+right_y)/2;
    seedv->setOffset(right_x,y,z);
    seedt->setOffset(left_x,y,z);
  }
  else if(type==2)
  {
    int y=(left_y+right_y)/2;
    seedv->setOffset(left_x,y,z);
    seedt->setOffset(right_x,y,z);
  }
  else if(type==3)
  {
    int x=(left_x+right_x)/2;
    seedv->setOffset(x,right_y,z);
    seedt->setOffset(x,left_y,z);
  }
  else
  {
    int x=(left_x+right_x)/2;
    seedv->setOffset(x,left_y,z);
    seedt->setOffset(x,right_y,z);
  }
}

void ZDownSamplingWindow::onRecover()
{
  if(!original)return;
  ZStack* zo=original->clone();

  ZStack* sampled=ZSandbox::GetCurrentDoc()->getStack();
  ZStack* recovered=recover2OriginalSize(original,sampled,step);

  /*process each plane*/
  for(int d=0;d<sampled->depth();++d)
  {
    std::vector<ZIntCuboid>boxes;
    std::vector<int>vvalue;
    std::vector<int>tvalue;
    std::vector<int>type;
    std::vector<ZStack*> seeds;

    ZStack* plane=sampled->makeCrop(ZIntCuboid(0,0,d,sampled->width()-1,sampled->height()-1,d));
    getBoundBoxes(plane,boxes,vvalue,tvalue,type,d);
    for(uint i=0;i<boxes.size();++i)//deal with each edge
    {
      const ZIntCuboid& box=boxes[i];
      Cuboid_I range=mapBox2OriginalSpace(box,original,step);
      generateSeedsInOriginalSpace(seeds,range,vvalue[i],tvalue[i],type[i]);
      localWaterShed(seeds,range,recovered,original);
      drawSeeds(seeds,zo,0);
      drawRange(zo,range,0);
      for(uint j=0;j<seeds.size();++j)
      {
         delete seeds[j];
      }
      seeds.clear();
    }
    delete plane;
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

#include <QAction>
#include <QWidget>
#include <QSpinBox>
#include <QMessageBox>
#include <vector>
#include <QPushButton>
#include <QGridLayout>
#include <cstdlib>
#include <fstream>
#include "zmultiscalewatershedmodule.h"
#include "imgproc/zstackwatershed.h"
#include "zstackdoc.h"
#include "zsandbox.h"
#include "mainwindow.h"
#include "neutubeconfig.h"
#include "zobject3dscan.hpp"
#include "imgproc/zstackmultiscalewatershed.h"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackdocdatabuffer.h"
#include "zstackframe.h"
#include "zcolorscheme.h"


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


void createTestStack()
{

  int w=400,h=400,d=400;
  ZStack* em=new ZStack(GREY,w,h,d,1);
  uint8_t* p=em->array8();
  int sx=100,sy=100,sz=100,r=100;
  int cl=210,cr=360,ct=300,cd=60,ci=400,co=0;
  for(int k=0;k<d;++k)
  {
    for(int j=0;j<h;++j)
    {
      for(int i=0;i<w;++i)
      {
        if(std::sqrt((k-sz)*(k-sz)+(j-sy)*(j-sy)+(i-sx)*(i-sx))<r-1)
        {
          p[k*w*h+j*w+i]=100+rand()%30;
        }
        else if(std::sqrt((k-sz)*(k-sz)+(j-sy)*(j-sy)+(i-sx)*(i-sx))<r+1)
        {
          p[k*w*h+j*w+i]=0;
        }
        else if(k>=co&&k<=ci&&j>=cd&&j<=ct&&i>=cl&&i<=cr)
        {
          p[k*w*h+j*w+i]=200+rand()%30;
        }
        else if(k>=co-1&&k<=ci+1&&j>=cd-1&&j<=ct+1&&i>=cl-1&&i<=cr+1)
        {
          p[k*w*h+j*w+i]=0;
        }
        else if(std::sqrt((k-100)*(k-100)+(j-300)*(j-300)+(i-60)*(i-60))<=101)
        {
          p[k*w*h+j*w+i]=150+rand()%30;
        }
        else
        {
          p[k*w*h+j*w+i]=0;
        }
      }
    }
  }
  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(em);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);
}

void ZWaterShedWindow::onOk()
{
#if 0
  const int size=16;
  //createTestStack();
  uint8_t data[]={1, 1, 8, 8, 7, 8, 7, 7, 8, 7, 8, 8 ,7 ,8 ,7 ,8,
                  7, 1, 1, 7, 7, 8, 6, 8, 7, 8, 8, 8, 8 ,7 ,7 ,7,
                  8, 8, 1, 7, 1, 7, 7, 7, 7, 8, 8, 6, 7, 8, 7, 8,
                  9, 7, 8, 6, 1, 1, 6, 8, 8, 7, 9, 7, 7, 7, 7, 7,
                  8, 8, 7, 7, 8, 1, 1, 7, 8, 8, 7, 8, 7, 8, 8, 8,
                  8, 8, 8, 7, 7, 8, 1, 1, 7, 1, 8, 7, 7, 7, 7, 7,
                  6, 7, 9, 9, 8, 7, 7, 7, 7, 1, 7, 8, 8, 6, 6, 8,
                  7, 8, 7, 8, 6, 6, 8, 8, 8, 1, 8, 9, 9, 7, 7, 7,
                  8, 9, 8, 7, 7, 7, 1, 1, 1, 1, 7, 8, 8, 8, 7, 8,
                  8, 8, 9, 6, 8, 1, 1, 8, 7, 8, 8, 7, 7, 9, 6, 7,
                  9, 7, 8, 7 ,7, 8, 1, 1, 1, 1, 8, 8 ,7, 9, 8, 7,
                  7, 8, 7, 8, 8, 8, 6, 8, 7, 8, 7, 8, 8, 8, 9, 6,
                  7, 8, 9, 6, 9, 7, 7, 7, 7, 1, 1, 8, 7, 7 ,7 ,8,
                  7, 7, 9, 7, 7, 8, 8, 6, 6, 7, 1, 8, 1, 1, 1, 7,
                  8, 7, 8, 8, 8, 8, 8, 7, 8, 8, 1, 7, 1, 8, 1, 1,
                  8, 9, 7, 9, 9, 7, 9, 7, 7, 8, 7, 8, 8, 9, 7, 1 };
  ZStack* src=new ZStack(GREY,16,16,1,1);
  for(int i=0;i<16*16;++i)
  {
    src->array8()[i]=data[i];
  }
  ZStack* seed=new ZStack(GREY,16,16,1,1);
  seed->array8()[15+0*16]=1;
  seed->array8()[0+15*16]=2;
  seed->array8()[5+1*16]=1;
  seed->array8()[3+5*16]=2;
  seed->array8()[11+3*16]=1;
  seed->array8()[6+7*16]=2;
  seed->array8()[11+11*16]=1;
  seed->array8()[11+15*16]=2;
  seed->array8()[13+7*16]=1;
  seed->array8()[6+12*16]=2;
  ZStack* result=ZStackWatershed().run(src,seed);
  for(int j=0;j<16;++j)
  {
     for(int i=0;i<16;++i)
     {
        std::cout<<(int)result->array8()[i+j*size]<<" ";
     }
     std::cout<<std::endl;
  }
#endif
#if 1
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  if(!src)return;
  int scale=spin_step->value();

  ZStackMultiScaleWatershed watershed;
  QList<ZSwcTree*> trees=doc->getSwcList();
  clock_t start=clock();
  ZStack* result=watershed.run(src,trees,scale);
  clock_t end=clock();
  std::cout<<"multiscale watershed total run time:"<<double(end-start)/CLOCKS_PER_SEC<<std::endl;

  if(result)
  {
    ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(src->clone());
//    ZSandbox::GetMainWindow()->addStackFrame(frame);
//    ZSandbox::GetMainWindow()->presentStackFrame(frame);

    std::vector<ZObject3dScan*> objArray =
        ZObject3dFactory::MakeObject3dScanPointerArray(*result);

//    ZStack* edge_obj=new ZStack(result->kind(),result->width(),result->height(),
//                                result->depth(),result->channelNumber());
//    frame=ZSandbox::GetMainWindow()->createStackFrame(edge_obj);

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

    frame=ZSandbox::GetMainWindow()->createStackFrame(result);
    ZSandbox::GetMainWindow()->addStackFrame(frame);
    ZSandbox::GetMainWindow()->presentStackFrame(frame);
    //delete result;
  }
#endif
}


void ZWaterShedWindow::onCancel()
{
  this->hide();
}


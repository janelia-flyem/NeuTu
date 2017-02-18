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


void ZWaterShedWindow::onOk()
{

  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  if(!src)return;
  int scale=spin_step->value();

  ZStackMultiScaleWatershed watershed;
  QList<ZSwcTree*> trees=doc->getSwcList();

  ZStack* result=watershed.run(src,trees,scale);

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

  //  frame=ZSandbox::GetMainWindow()->createStackFrame(result);
  //  ZSandbox::GetMainWindow()->addStackFrame(frame);
  //  ZSandbox::GetMainWindow()->presentStackFrame(frame);
    delete result;
  }
}


void ZWaterShedWindow::onCancel()
{
  this->hide();
}


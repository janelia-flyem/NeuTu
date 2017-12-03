#include <QAction>
#include <QWidget>
#include <QSpinBox>
#include <QMessageBox>
#include <QTime>
#include <vector>
#include <map>
#include <QPushButton>
#include <QGridLayout>
#include <cstdlib>
#include <fstream>
#include "zmultiscalewatershedmodule.h"
#include "zstackdoc.h"
#include "zsandbox.h"
#include "mainwindow.h"
#include "neutubeconfig.h"
#include "zobject3dscan.hpp"
#include "zobject3dfactory.h"
#include "zobject3darray.h"
#include "zstackdocdatabuffer.h"
#include "zstackframe.h"
#include "zcolorscheme.h"
#include "zobject3dscanarray.h"
#include "flyem/zstackwatershedcontainer.h"

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
  algorithms=new QComboBox;
  algorithms->addItem("watershed");
  algorithms->addItem("random_walker");
  algorithms->addItem("power_watershed");
  algorithms->addItem("MSF_Cruskal");
  algorithms->addItem("MSF_Prime");
  lay->addWidget(algorithms,1,0,1,2);
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

  doc->removeObject(ZStackObjectRole::ROLE_SEGMENTATION);

  ZStackWatershedContainer container(src);
  int i=0;
  for(ZSwcTree* tree:doc->getSwcArray()){
      tree->setType(++i);
      container.addSeed(*tree);
  }
  std::map<QString,int> color_indices;
  for(ZStroke2d* stroke:doc->getStrokeList()){
      std::map<QString,int>::iterator p_index=color_indices.find(stroke->getColor().name());
      if (p_index==color_indices.end()){
          color_indices.insert(std::make_pair(stroke->getColor().name(),++i));
          stroke->setLabel(i);
      }
      else{
          stroke->setLabel(p_index->second);
      }
      container.addSeed(*stroke);
  }
  container.setScale(scale);
#ifdef _DEBUG_
  QTime time;
  time.start();
#endif
  container.setAlgorithm(algorithms->currentText());
  container.run();

  std::cout<<"+++++++++++++multiscale watershed total run time:"<<time.elapsed()/1000.0<<std::endl;

  ZObject3dScanArray result;
  container.makeSplitResult(1, &result);
  for (ZObject3dScanArray::iterator iter = result.begin();
       iter != result.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    doc->getDataBuffer()->addUpdate(
          obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
  }
  doc->getDataBuffer()->deliver();
  result.shallowClear();

#if 0
=======
  ZStack* result=container.getResultStack()->clone();

  if(result){
    ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(src->clone());
//    ZSandbox::GetMainWindow()->addStackFrame(frame);
//    ZSandbox::GetMainWindow()->presentStackFrame(frame);

    std::vector<ZObject3dScan*> objArray =
        ZObject3dFactory::MakeObject3dScanPointerArray(*result, 1, false);

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


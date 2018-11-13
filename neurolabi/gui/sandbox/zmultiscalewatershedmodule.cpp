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
#include "zsparsestack.h"
#include "flyem/zstackwatershedcontainer.h"
#include "zstack.hxx"

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
  lay->addWidget(ok,1,4);

  algorithms=new QComboBox;
  algorithms->addItem("watershed");
  algorithms->addItem("random_walker");
  algorithms->addItem("power_watershed");
  algorithms->addItem("MSF_Cruskal");
  algorithms->addItem("MSF_Prime");
  algorithms->addItem("FFN");
  lay->addWidget(algorithms,1,0,1,2);

  ds_method=new QComboBox;
  ds_method->addItem("Min");
  ds_method->addItem("Min(ignore zero)");
  ds_method->addItem("Max");
  ds_method->addItem("Mean");
  ds_method->addItem("Edge");

  lay->addWidget(ds_method,1,2,1,2);
  this->setLayout(lay);
  this->move(300,200);
  connect(ok,SIGNAL(clicked()),this,SLOT(onOk()));
}


void ZWaterShedWindow::onOk()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  ZSparseStack* spSrc=doc->getSparseStack();
  if(!src && !spSrc)return;

  int scale=spin_step->value();
  doc->removeObject(ZStackObjectRole::ROLE_SEGMENTATION);
  ZStackWatershedContainer container(src,spSrc);

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
  container.setDsMethod(ds_method->currentText());
  container.setAlgorithm(algorithms->currentText());

  QTime time;
  time.start();
  container.run();
  /*size_t cnt=0;

  for(size_t i=0;i<src->getVoxelNumber();++i){
    if(src->array8()[i])cnt++;
  }
  std::cout<<"+++++++++++++cnt:"<<cnt<<std::endl;*/
  std::cout<<"+++++++++++++multiscale watershed total run time:"<<time.elapsed()/1000.0<<std::endl;

  ZObject3dScanArray result;
  container.makeSplitResult(1, &result, NULL);
  for (ZObject3dScanArray::iterator iter = result.begin();
       iter != result.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    doc->getDataBuffer()->addUpdate(
          obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
  }
  doc->getDataBuffer()->deliver();
  result.shallowClear();
  ZStackFrame* frame=ZSandbox::GetMainWindow()->createStackFrame(
        container.getResultStack()->clone());
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);

}


void ZWaterShedWindow::onCancel()
{
  this->hide();
}


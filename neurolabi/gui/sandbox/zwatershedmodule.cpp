#include<QAction>
#include<QMessageBox>
#include<vector>
#include"zwatershedmodule.h"
#include"zstackwatershed.h"
#include"zstackdoc.h"
#include"zsandbox.h"
#include"mainwindow.h"


ZWaterShedModule::ZWaterShedModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


ZWaterShedModule::~ZWaterShedModule()
{

}


void ZWaterShedModule::init()
{
  m_action = new QAction("WaterShedSegmentation", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}


void ZWaterShedModule::execute()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  ZStack    *signal,*seed,*result;
  if((!doc) || !(signal=doc->getStack()))
  {
    return;
  }

  std::vector<ZStack*> seeds;
  QList<ZSwcTree*> trees=doc->getSwcList();
  if(trees.size()<2 || trees.size()>255)
  {
    return ;
  }
  uint scale=trees.size()==2?2:1;
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
    tree->labelStack(seed,scale*seed_index++);

    seeds.push_back(seed);
  }

  ZStackWatershed watershed;
  /*set range of area involved in calculation
   *and run the watershed algorithm*/

  /*Cuboid_I range;
  Cuboid_I_Set_S(&range,0,0,125,1024,1024,1);
  watershed.setRange(range);*/
  result=watershed.run(signal,seeds);

  /*create a frame of result to show in the main window*/
  if(result)
  {
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


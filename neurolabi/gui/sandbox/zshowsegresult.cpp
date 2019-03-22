#include <vector>
#include <iostream>
#include <QFileDialog>
#include "zshowsegresult.h"
#include "mvc/zstackdoc.h"
#include "mvc/zstackframe.h"
#include "zobject3d.h"
#include "zobject3dfactory.h"
#include "zcolorscheme.h"
#include "zsandbox.h"
#include "mainwindow.h"
#include "zstackdocdatabuffer.h"
#include "zstack.hxx"
#include "zobject3dscan.h"

ZShowSegResultModule::ZShowSegResultModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZShowSegResultModule::init()
{
  m_action = new QAction("ShowSegResult", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}


void ZShowSegResultModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL){
    ZStack* stack=doc->getStack();
    if(stack!=NULL){
      QString name=QFileDialog::getOpenFileName(NULL,"SegmentationStack","*.tif");
      ZStack* result=new ZStack();
      result->load(name.toStdString());
      result->setOffset(stack->getOffset());
      ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(stack->clone());
      std::vector<ZObject3dScan*> objArray =ZObject3dFactory::MakeObject3dScanPointerArray(*result, 1, true);
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
                obj,ZStackDocObjectUpdate::EAction::ADD_UNIQUE);
          frame->document()->getDataBuffer()->deliver();
        }
      }
      delete result;
      ZSandbox::GetMainWindow()->addStackFrame(frame);
      ZSandbox::GetMainWindow()->presentStackFrame(frame);
    }
  }
}

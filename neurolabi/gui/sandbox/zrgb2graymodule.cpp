#include "zrgb2graymodule.h"

#include <QAction>

#include "zsandbox.h"
#include "mvc/zstackdoc.h"
#include "imgproc/zstackprocessor.h"
#include "mainwindow.h"
#include "zstack.hxx"

ZRgb2GrayModule::ZRgb2GrayModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZRgb2GrayModule::init()
{
  m_action = new QAction("RGB->Gray", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}

void ZRgb2GrayModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL) {
    ZStack *stack = doc->getStack();
    if (stack->channelNumber() == 3) {
      ZStack *newStack = ZStackProcessor::Rgb2Gray(stack);
      if (newStack != NULL) {
        ZStackFrame *frame =
            ZSandbox::GetMainWindow()->createStackFrame(newStack);
        ZSandbox::GetMainWindow()->addStackFrame(frame);
        ZSandbox::GetMainWindow()->presentStackFrame(frame);
      }
    }
  }
}


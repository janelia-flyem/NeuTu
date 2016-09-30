#include "zsandbox.h"

#include <QMenu>

#include "QsLog/QsLog.h"
#include "mainwindow.h"
#include "zstackframe.h"
#include "zsandboxmodule.h"

ZSandbox::ZSandbox()
{
  init();
}

void ZSandbox::init()
{
  m_mainWin = NULL;
}


void ZSandbox::setMainWindow(MainWindow *win)
{
  m_mainWin = win;
}

void ZSandbox::SetMainWindow(MainWindow *win)
{
  GetInstance().setMainWindow(win);
}

MainWindow* ZSandbox::getMainWindow() const
{
  return m_mainWin;
}

ZStackFrame* ZSandbox::getCurrentFrame() const
{
  ZStackFrame *frame = NULL;
  if (getMainWindow() != NULL) {
    frame = getMainWindow()->currentStackFrame();
  }

  return frame;
}

ZStackDoc* ZSandbox::getCurrentDoc() const
{
  ZStackDoc *doc = NULL;
  ZStackFrame *frame = getCurrentFrame();
  if (frame != NULL) {
    doc = frame->document().get();
  }

  return doc;
}

MainWindow* ZSandbox::GetMainWindow()
{
  return GetInstance().getMainWindow();
}

ZStackFrame* ZSandbox::GetCurrentFrame()
{
  return GetInstance().getCurrentFrame();
}

ZStackDoc* ZSandbox::GetCurrentDoc()
{
  return GetInstance().getCurrentDoc();
}

void ZSandbox::registerModule(ZSandboxModule *module)
{
  module->setParent(GetMainWindow());

  if (module->getMenu() != NULL) {
    GetMainWindow()->getSandboxMenu()->addMenu(module->getMenu());
  } else if (module->getAction() != NULL) {
    GetMainWindow()->getSandboxMenu()->addAction(module->getAction());
  } else {
    LWARN() << "Failed to register a module.";
  }
}

void ZSandbox::RegisterModule(ZSandboxModule *module)
{
  GetInstance().registerModule(module);
}

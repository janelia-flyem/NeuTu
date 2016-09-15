#include "zaboutmodule.h"
#include <QAction>
#include <QMessageBox>
#include "zsandbox.h"
#include "mainwindow.h"

ZAboutModule::ZAboutModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

QAction* ZAboutModule::getAction() const
{
  return m_action;
}

void ZAboutModule::init()
{
  m_action = new QAction("About", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}

void ZAboutModule::execute()
{
  QMessageBox::about(ZSandbox::GetMainWindow(), "About Sandbox",
                     "NeuTu sandbox. Designed by Ting Zhao.");
}




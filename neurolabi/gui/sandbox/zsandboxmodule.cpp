#include "zsandboxmodule.h"
#include "mainwindow.h"

ZSandboxModule::ZSandboxModule(QObject *parent) :
  QObject(parent)
{
  init();
}

void ZSandboxModule::init()
{
  m_action = NULL;
  m_menu = NULL;
}

/*
void ZSandboxModule::initialize(MainWindow *mainWindow)
{
  if (mainWindow != NULL) {
    connect(this, SIGNAL(docGenerated(ZStackDoc*)),
            mainWindow, SLOT(showStackDoc(ZStackDoc*)));
  }
}
*/

QMenu* ZSandboxModule::getMenu() const
{
  return m_menu;
}

QAction* ZSandboxModule::getAction() const
{
  return m_action;
}

void ZSandboxModule::setMenu(QMenu *menu)
{
  m_menu = menu;
}

void ZSandboxModule::setAction(QAction *action)
{
  m_action = action;
}

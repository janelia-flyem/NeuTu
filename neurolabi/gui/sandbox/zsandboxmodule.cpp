#include "zsandboxmodule.h"

ZSandboxModule::ZSandboxModule(QObject *parent) :
  QObject(parent)
{
}

QMenu* ZSandboxModule::getMenu() const
{
  return NULL;
}

QAction* ZSandboxModule::getAction() const
{
  return NULL;
}

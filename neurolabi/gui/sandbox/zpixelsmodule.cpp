#include "zpixelsmodule.h"
#include<QMessageBox>
#include <QAction>
#include<QString>
#include "zsandbox.h"
#include "zstackdoc.h"
#include "zstackprocessor.h"

ZPixelsModule::ZPixelsModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZPixelsModule::init()
{
  m_action = new QAction("Pixels", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}

void ZPixelsModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL)
  {
      int height=doc->getStackHeight();
      int width=doc->getStackWidth();
      QMessageBox::about
              ((QWidget*)ZSandbox::GetMainWindow(), "Pixels",
               "height: "+QString::number(height)+"\n"
               +"width: "+QString::number(width)+"\n"
               +"total pixels: "+QString::number(height*width)
               );
  }
}

QAction* ZPixelsModule::getAction() const
{
  return m_action;
}

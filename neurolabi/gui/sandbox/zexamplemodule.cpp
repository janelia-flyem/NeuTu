#include "zexamplemodule.h"

#include <QAction>

#include "zsandbox.h"
#include "mvc/zstackdoc.h"
#include "imgproc/zstackprocessor.h"

ZExampleModule::ZExampleModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZExampleModule::init()
{
  QAction *action = new QAction("Example", this);
  connect(action, SIGNAL(triggered()), this, SLOT(execute()));

  setAction(action);
}

void ZExampleModule::execute()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL) {
    if (doc->hasStackData()) {
      ZStackProcessor::SubtractBackground(doc->getStack());
      doc->notifyStackModified(false);
    }
  }
}

#include "zexamplemodule.h"

#include <QAction>

#include "zsandbox.h"
#include "zstackdoc.h"
#include "zstackprocessor.h"

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
    ZStack *stack = doc->getStack();
    if (stack->hasData()) {
      ZStackProcessor::SubtractBackground(stack);
      doc->notifyStackModified();
    }
  }
}

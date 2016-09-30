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
  m_action = new QAction("Example", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
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

QAction* ZExampleModule::getAction() const
{
  return m_action;
}

#include "ztracemodule.h"

#include <QMenu>

#include "zsandbox.h"
#include "mvc/zstackdoc.h"
#include "zneurontracer.h"
#include "zswcfactory.h"
#include "zswctree.h"

ZTraceModule::ZTraceModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


void ZTraceModule::init()
{
  QMenu *menu = new QMenu("Trace");
  setMenu(menu);

  QAction *seedAction = new QAction("Compute Seeds", this);
  connect(seedAction, SIGNAL(triggered()), this, SLOT(computeSeed()));

  getMenu()->addAction(seedAction);
}

void ZTraceModule::computeSeed()
{
  ZStackDoc *doc = ZSandbox::GetCurrentDoc();
  if (doc != NULL) {
    ZNeuronTracer &tracer = doc->getNeuronTracer();
    std::vector<ZWeightedPoint> ptArray = tracer.computeSeedPosition();
    ZSwcTree *tree = ZSwcFactory::CreateSwc(ptArray);
    tree->setSource("ZTraceModule::computeSeed");
    doc->addObject(tree, true);
  }
}


#include "zstackdocobjectmonitor.h"

#include "zstackobjectinfo.h"
#include "zstackdoc.h"

ZStackDocObjectMonitor::ZStackDocObjectMonitor(QObject *parent) : QObject(parent)
{
  m_processObjectModified = [](const ZStackObjectInfoSet &infoSet) {
    infoSet.print();
  };
}

void ZStackDocObjectMonitor::processObjectModified(
    const ZStackObjectInfoSet &infoSet)
{
  if (m_processObjectModified) {
    m_processObjectModified(infoSet);
  }
}

void ZStackDocObjectMonitor::monitor(ZStackDoc *doc)
{
  connect(doc, SIGNAL(objectModified(ZStackObjectInfoSet)),
          this, SLOT(processObjectModified(ZStackObjectInfoSet)));
}

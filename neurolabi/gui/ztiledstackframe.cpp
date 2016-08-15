#include "ztiledstackframe.h"

#include <QMdiArea>

#include "ztilemanager.h"
#include "ztilegraphicsitem.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zstackview.h"
#include "zstackobjectsourcefactory.h"
#include "z3dgraphfactory.h"

ZTiledStackFrame::ZTiledStackFrame(QWidget *parent) :
  ZStackFrame(parent)
{
  m_tileManager = new ZTileManager(this);
}

ZTiledStackFrame*
ZTiledStackFrame::Make(QMdiArea *parent)
{
  return Make(parent, ZSharedPointer<ZStackDoc>(new ZStackDoc));
}

ZTiledStackFrame *ZTiledStackFrame::Make(
    QMdiArea *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZTiledStackFrame *frame = new ZTiledStackFrame(parent);

  BaseConstruct(frame, doc);

//  doc->setResolution(1, 1, 8, 'p');

  return frame;
}


bool ZTiledStackFrame::importTiles(const QString &path)
{
  Q_ASSERT(m_doc.get() != NULL);

  if (m_tileManager->importJsonFile(path)) {
    /*
    std::string filePath =
        m_tileManager->getFirstTile()->getTileInfo().getSource();
        */
   m_tileManager->selectItem(m_tileManager->getFirstTile());
    /*
    m_doc->readStack(filePath.c_str(), false);
    setWindowTitle(filePath.c_str());
    m_statusInfo =  QString("%1 loaded").arg(filePath.c_str());
    m_presenter->optimizeStackBc();
    m_view->reset();
    */
    return true;
  }

  return false;
}

void ZTiledStackFrame::updateStackBoundBox()
{
  ZIntCuboid box = document()->getStack()->getBoundBox();

  Z3DGraphFactory factory;
  factory.setShapeHint(GRAPH_LINE);
  factory.setNodeRadiusHint(0);
  factory.setEdgeWidthHint(1.0);
  factory.setEdgeColorHint(QColor(200, 0, 200));
  Z3DGraph *graph = factory.makeBox(box);
  graph->setSource(ZStackObjectSourceFactory::MakeStackBoundBoxSource());
  document()->addObject(graph);
}

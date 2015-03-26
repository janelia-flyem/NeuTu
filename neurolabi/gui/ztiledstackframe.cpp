#include "ztiledstackframe.h"

#include <QMdiArea>

#include "ztilemanager.h"
#include "ztilegraphicsitem.h"
#include "zstackdoc.h"
#include "zstackpresenter.h"
#include "zstackview.h"

ZTiledStackFrame::ZTiledStackFrame(QWidget *parent) :
  ZStackFrame(parent)
{
  m_tileManager = new ZTileManager(this);
}

ZTiledStackFrame*
ZTiledStackFrame::Make(QMdiArea *parent)
{
  return Make(parent, ZSharedPointer<ZStackDoc>(
                new ZStackDoc(NULL, NULL)));
}

ZTiledStackFrame *ZTiledStackFrame::Make(
    QMdiArea *parent, ZSharedPointer<ZStackDoc> doc)
{
  ZTiledStackFrame *frame = new ZTiledStackFrame(parent);

  BaseConstruct(frame, doc);

  return frame;
}


bool ZTiledStackFrame::importTiles(const QString &path)
{
  Q_ASSERT(m_doc != NULL);

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

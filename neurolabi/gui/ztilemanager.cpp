#include "ztilemanager.h"
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "ztileinfo.h"
#include "ztilegraphicsitem.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "zstack.hxx"
#include "zstackpresenter.h"
#include "zstackview.h"
#include "zpoint.h"
#include "ztiledstackframe.h"

#include <QFileInfo>

const QColor ZTileManager::m_selectionColor = QColor(255, 0, 0);
//const QColor ZTileManager::m_preselectionColor = QColor(0, 255, 0);

ZTileManager::ZTileManager(QObject *parent) : QGraphicsScene(parent),
  m_selectedTileItem(NULL), /*m_preselected(NULL),*/ m_highlightRec(NULL), m_view(NULL)/*, m_selectDecoration(NULL)*/
{
//    scaleFactor = 1.0;
    getParentFrame()->setTileManager(this);
    m_resolution.set(1, 1, 8, 'p'); //default setup for biocytin data
}

ZTileManager::~ZTileManager()
{
}

void ZTileManager::initDecoration()
{
  /*
  m_selectDecoration = new QGraphicsRectItem;
  m_selectDecoration->setFlag(QGraphicsItem::ItemIsSelectable, false);
  m_selectDecoration->setZValue(-1.0); //always on top
  m_selectDecoration->setPen(QPen(QColor(255, 0, 0)));
  addItem(m_selectDecoration);
  */
}

ZTileGraphicsItem* ZTileManager::getFirstTile()
{
  QList<QGraphicsItem*> itemList = items();
  foreach (QGraphicsItem *item, itemList) {
    ZTileGraphicsItem *firstItem = dynamic_cast<ZTileGraphicsItem*>(item);
    if (firstItem != NULL) {
      return firstItem;
    }
  }

  return NULL;
}

bool ZTileManager::importJsonFile(const QString &filePath)
{
  clear();

  bool succ = false;
  if (!filePath.isEmpty()) { 
    ZJsonObject obj;
    obj.load(filePath.toStdString());

    //get the json file path
    QFileInfo fInfo(filePath);
    QString tileFilePath = fInfo.absolutePath();

    if (obj.hasKey("Tiles")) {
      json_t *value = obj["Tiles"];
      if (ZJsonParser::isArray(value)) {
        ZJsonArray array(value, false);
        for (size_t i = 0; i < array.size(); ++i) {
          ZJsonObject tileObj(array.at(i), false);
          if (!tileObj.isEmpty()) {
            ZTileGraphicsItem *tileItem = new ZTileGraphicsItem;
            if (tileItem->loadJsonObject(tileObj,tileFilePath)) {
              //tileItem->setFlag(QGraphicsItem::ItemIsSelectable);
//              tileItem->setScale(scaleFactor);
              addItem(tileItem);
              succ = true;
            } else {
              delete tileItem;
            }
          }
        }
      }
    }

    if (obj.hasKey("Metadata")) {
      ZJsonObject metaObj(obj.value("Metadata"));
      if (metaObj.hasKey("Resolution")) {
        ZJsonObject resObj(metaObj.value("Resolution"));

        m_resolution.loadJsonObject(resObj);
      }
    }
  }

  //std::cout << items().size() << " tiles" << std::endl;

  if (succ) {
    if (getFirstTile() != NULL) {
      m_selectedTileItem = NULL;
      //m_preselected = NULL;
      initDecoration();
    } else {
      succ = false;
    }
  }

  return succ;
}

ZStackFrame* ZTileManager::getParentFrame() const
{
  return qobject_cast<ZStackFrame*>(parent());
}

void ZTileManager::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
  ZTileGraphicsItem* hitItem =
      dynamic_cast<ZTileGraphicsItem*>(
        itemAt(event->scenePos().x(), event->scenePos().y(), QTransform()));

  if (hitItem != NULL) {
      //clearPreselected();
      selectItem(hitItem);
      //preselectItem(hitItem);
  }
}

/*
void ZTileManager::clearPreselected()
{
  if (m_preselected != NULL) {
    m_preselected = NULL;
  }
}
*/


void ZTileManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  UNUSED_PARAMETER(event);
    if (m_view != NULL) getParentView()->viewport()->update();
/*
    clearPreselected();

  ZTileGraphicsItem* hitItem =
      dynamic_cast<ZTileGraphicsItem*>(
        itemAt(event->scenePos().x(), event->scenePos().y(), QTransform()));

  if (hitItem != NULL) {
    selectItem(hitItem);
  }
  */
}

void ZTileManager::updateTileStack()
{
  ZStackFrame *frame = getParentFrame();
  if (frame != NULL && m_selectedTileItem != NULL) {
    std::string source = m_selectedTileItem->getTileInfo().getSource();
    //plot boundary of the selected tile.
    plotItemBoundary(m_selectedTileItem,m_selectionColor);
    if (source != std::string(frame->document()->stackSourcePath())) {
      startProgress();
      advanceProgress(0.5);
      QApplication::processEvents();
      frame->document()->readStack(source.c_str(), false);

      frame->document()->setStackOffset(
            m_selectedTileItem->getTileInfo().getOffset());

      frame->document()->setResolution(m_resolution);

      if (GET_APPLICATION_NAME == "Biocytin") {
        frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
        frame->autoBcAdjust();
        frame->loadRoi(true);

        ZTiledStackFrame *completeFrame = qobject_cast<ZTiledStackFrame*>(frame);
        if (completeFrame != NULL) {
          completeFrame->updateStackBoundBox();
        }
      }
      frame->setWindowTitle(source.c_str());
      endProgress();
    }
  }
}

void ZTileManager::plotItemBoundary(ZTileGraphicsItem *item, QColor boundaryColor)
{
    if (m_highlightRec != NULL) removeItem(m_highlightRec);
    m_highlightRec = addRect(item->mapRectToParent(item->boundingRect()),boundaryColor);
}

/*
void ZTileManager::preselectItem(ZTileGraphicsItem *item)
{
  if (m_preselected != item && m_selectedTileItem != item && item != NULL) {
    m_preselected = item;
  }
}
*/

void ZTileManager::selectItem(ZTileGraphicsItem *item)
{
  if (m_selectedTileItem != item && item != NULL) {
    //if (m_selectedTileItem != NULL) {
        //erase previous boundary
    //    removeItem(m_highlightRec);
    //}
    m_selectedTileItem = item;
    /*
    if (m_selectedTileItem == m_preselected) {
      m_preselected = NULL;
    }
    */
    //m_selectDecoration->setRect(m_selectedTileItem->rect());
    //qDebug() << m_selectDecoration->rect();
//    emit(loadingTile());
    updateTileStack();
  }
}

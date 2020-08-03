#include "zflyemtodolist.h"

#include "common/math.h"
#include "zpainter.h"
#include "geometry/zgeometry.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidglobal.h"

#include "mvc/zstackview.h"
#include "flyemdatareader.h"

ZFlyEmToDoList::ItemSlice
ZFlyEmToDoList::m_emptySlice(ZFlyEmToDoList::STATUS_NULL);
ZFlyEmToDoItem ZFlyEmToDoList::m_emptyTodo;


ZFlyEmToDoList::ZFlyEmToDoList()
{
  m_type = GetType();
  init();
}

ZFlyEmToDoList::~ZFlyEmToDoList()
{

}


void ZFlyEmToDoList::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (m_reader.open(target)) {
    m_writer.open(target);
    m_dvidInfo = ZDvidGlobal::Memo::ReadGrayscaleInfo(m_reader.getDvidTarget());
//    m_dvidInfo = m_reader.readGrayScaleInfo();
    m_startZ = m_dvidInfo.getStartCoordinates().getCoord(m_sliceAxis);
  }
}

void ZFlyEmToDoList::setDvidInfo(const ZDvidInfo &info)
{
  m_dvidInfo = info;
  m_startZ = m_dvidInfo.getStartCoordinates().getCoord(m_sliceAxis);
}

void ZFlyEmToDoList::init()
{
  m_startZ = 0;
  m_view = NULL;
  m_maxPartialArea = 1024 * 1024;
  m_sliceAxis = neutu::EAxis::Z;
  m_isReady = false;
}

void ZFlyEmToDoList::clearCache()
{
  m_itemList.clear();
}

ZIntCuboid ZFlyEmToDoList::update(const ZIntCuboid &box)
{
  ZIntCuboid dataBox = box;
  if (!m_dataRange.isEmpty()) {
    dataBox.intersect(m_dataRange);
  }

  if (!dataBox.isEmpty()) {
    ZDvidUrl dvidUrl(m_dvidTarget);
    ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getTodoListUrl(dataBox));

    for (size_t i = 0; i < obj.size(); ++i) {
      ZJsonObject itemJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (itemJson.hasKey("Pos")) {
        ZFlyEmToDoItem item;
        item.loadJsonObject(itemJson, dvid::EAnnotationLoadMode::PARTNER_RELJSON);
        addItem(item, DATA_LOCAL);
      }
    }
  }

  return dataBox;
}

void ZFlyEmToDoList::update(int x, int y, int z)
{
  ZFlyEmToDoItem item = FlyEmDataReader::ReadToDoItem(m_reader, x, y, z);
  if (item.isValid()) {
    addItem(item, DATA_LOCAL);
  } else {
    removeItem(x, y, z, DATA_LOCAL);
  }
}

void ZFlyEmToDoList::update(const ZIntPoint &pt)
{
  update(pt.getX(), pt.getY(), pt.getZ());
}

void ZFlyEmToDoList::update(const std::vector<ZIntPoint> &ptArray)
{
  for (const ZIntPoint &pt : ptArray) {
    update(pt);
  }
}

void ZFlyEmToDoList::attachView(ZStackView *view)
{
  m_view = view;
}

void ZFlyEmToDoList::download(int z)
{
  int currentArea = 0;
  if (m_view != NULL) {
    currentArea = m_view->getViewParameter().getArea(neutu::data3d::ESpace::MODEL);
  }

  int blockIndex = m_dvidInfo.getBlockIndexZ(z);
  ZIntCuboid blockBox =
      m_dvidInfo.getBlockBox(blockIndex, blockIndex, blockIndex);
  blockBox.shiftSliceAxis(m_sliceAxis);

  if (currentArea > 0 && currentArea < m_maxPartialArea) {
    ZAffineRect viewPort = m_view->getCutRect();
    ZIntCuboid box = zgeom::GetIntBoundBox(viewPort);
    box.setMinZ(blockBox.getMinCorner().getZ());
    box.setMaxZ(blockBox.getMaxCorner().getZ());
//    QRect viewPort = m_view->getViewParameter().getViewPort();
//    ZIntCuboid box(
//          viewPort.left(), viewPort.top(), blockBox.getMinCorner().getZ(),
//          viewPort.right(), viewPort.bottom(), blockBox.getMaxCorner().getZ());
    box.shiftSliceAxisInverse(m_sliceAxis);
    update(box);
    for (int cz = blockBox.getMinCorner().getZ();
         cz <= blockBox.getMaxCorner().getZ(); ++cz) {
      ItemSlice &slice = getSlice(cz, ADJUST_FULL);
      slice.setDataRect(viewPort);
      slice.setStatus(STATUS_PARTIAL_READY);
    }
  } else if (m_fetchingFullAllowed) {
    ZIntPoint lastCorner = m_dvidInfo.getEndCoordinates();
    ZIntPoint firstCorner = m_dvidInfo.getStartCoordinates();

    firstCorner.shiftSliceAxis(m_sliceAxis);
    lastCorner.shiftSliceAxis(m_sliceAxis);

    int width = lastCorner.getX() - firstCorner.getX() + 1;
    int height = lastCorner.getY() - firstCorner.getY() + 1;
    ZIntCuboid box;

    box.setMinCorner(firstCorner.getX(), firstCorner.getY(),
                       blockBox.getMinCorner().getZ());
    box.setSize(width, height, blockBox.getDepth());

    box.shiftSliceAxisInverse(m_sliceAxis);

    box = update(box);

    for (int cz = blockBox.getMinCorner().getZ();
         cz <= blockBox.getMaxCorner().getZ(); ++cz) {
      ItemSlice &slice = getSlice(cz, ADJUST_FULL);
      if (m_dataRange.isEmpty()) {
        slice.setStatus(STATUS_READY);
      } else {
        box.shiftSliceAxisInverse(getSliceAxis());
        ZAffineRect rect;
        rect.setCenter(box.getExactCenter());
        rect.setSize(box.getWidth(), box.getHeight());
//        slice.setDataRect(
//              QRect(box.getMinCorner().getX(), box.getMinCorner().getY(),
//                    box.getWidth(), box.getHeight()));
        slice.setStatus(STATUS_PARTIAL_READY);
      }
    }
  }
}

/*
void ZFlyEmToDoList::downloadForLabel(uint64_t label)
{
  ZDvidUrl dvidUrl(m_dvidTarget);
  ZJsonArray obj = m_reader.readJsonArray(dvidUrl.getTodoListUrl(label, false));

  for (size_t i = 0; i < obj.size(); ++i) {
    ZJsonObject itemJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    ZFlyEmToDoItem item;
    item.loadJsonObject(itemJson, FlyEM::LOAD_PARTNER_RELJSON);
    if (item.isValid()) {
      addItem(item, DATA_LOCAL);
    }
  }
}
*/
ZFlyEmToDoList::ItemSlice& ZFlyEmToDoList::getSlice(int z, EAdjustment adjust)
{
  if (adjust == ADJUST_NONE) {
    return getSlice(z);
  }

  int zIndex = z - m_startZ;
  if (m_itemList.size() <= zIndex) {
    m_itemList.resize(zIndex + 1);
  } else if (zIndex < 0) {
    if (adjust == ADJUST_FULL) {
      if (zIndex < 0) {
        QVector<ItemSlice> se = m_itemList;
        m_itemList.clear();
        m_itemList.resize(se.size() - zIndex);
        for (int i = 0; i < se.size(); ++i) {
          m_itemList[i - zIndex] = se[i];
        }
        m_startZ = z;
        zIndex = 0;
      }
    }
  }


  return m_itemList[zIndex];
}

const ZFlyEmToDoList::ItemSlice& ZFlyEmToDoList::getSlice(int z) const
{
  int zIndex = z - m_startZ;

  if (zIndex >= 0 && zIndex < m_itemList.size()) {
    return m_itemList[zIndex];
  }

  return m_emptySlice;
}

ZFlyEmToDoList::ItemSlice& ZFlyEmToDoList::getSlice(int z)
{
  return const_cast<ZFlyEmToDoList::ItemSlice&>(
        static_cast<const ZFlyEmToDoList&>(*this).getSlice(z));
}

ZFlyEmToDoList::ItemMap &ZFlyEmToDoList::getItemMap(
    int y, int z, EAdjustment adjust)
{
  ItemSlice &slice = getSlice(z, adjust);

  return slice.getMap(y, adjust);
}

const ZFlyEmToDoList::ItemMap &ZFlyEmToDoList::getItemMap(int y, int z) const
{
  const ZFlyEmToDoList::ItemSlice &slice = getSlice(z);

  return slice.getMap(y);
}

ZFlyEmToDoList::ItemMap &ZFlyEmToDoList::getItemMap(int y, int z)
{
  return const_cast<ZFlyEmToDoList::ItemMap&>(
        static_cast<const ZFlyEmToDoList&>(*this).getItemMap(y, z));
}

int ZFlyEmToDoList::getMinZ() const
{
  return m_startZ;
}

int ZFlyEmToDoList::getMaxZ() const
{
  return m_startZ + m_itemList.size() - 1;
}

bool ZFlyEmToDoList::hasLocalItem(int x, int y, int z) const
{
  zgeom::ShiftSliceAxis(x, y, z, m_sliceAxis);

  int zIndex = z - m_startZ;


  if (zIndex < 0 || zIndex >= m_itemList.size()) {
    return false;
  }

  const ItemSlice &slice = getSlice(z);

  return slice.contains(x, y);
}

bool ZFlyEmToDoList::removeItem(
    const ZIntPoint &pt, EDataScope scope)
{
  return removeItem(pt.getX(), pt.getY(), pt.getZ(), scope);
}

bool ZFlyEmToDoList::removeItem(int x, int y, int z, EDataScope scope)
{
  if (scope == ZFlyEmToDoList::DATA_LOCAL) {
    if (hasLocalItem(x, y, z)) {
      int sx = x;
      int sy = y;
      int sz = z;
      zgeom::ShiftSliceAxis(sx, sy, sz, m_sliceAxis);
      getItemMap(sy, sz).remove(sx);
      getSelector().deselectObject(ZIntPoint(x, y, z));

      return true;
    }
  } else {
    ZDvidWriter &writer = m_writer;
    if (writer.good()) {
      writer.deleteToDoItem(x, y, z);
    }

    if (writer.isStatusOk()) {
      return removeItem(x, y, z, DATA_LOCAL);
    }
  }

  return false;
}

void ZFlyEmToDoList::addItem(
    const ZFlyEmToDoItem &item, EDataScope scope)
{
  if (scope == DATA_LOCAL) {
    ZIntPoint center = item.getPosition();
    center.shiftSliceAxis(m_sliceAxis);
    ItemMap &itemMap =
        getItemMap(center.getY(), center.getZ(), ADJUST_FULL);

    ZFlyEmToDoItem &targetItem = itemMap[center.getX()];
    if (!targetItem.isSelected() && item.isSelected()) {
      getSelector().selectObject(item.getPosition());
    }

    bool isSelected = targetItem.isSelected() || item.isSelected();
    targetItem = item;

    if (isSelected) {
      targetItem.setSelected(isSelected);
    }
  } else {
    ZDvidWriter &writer = m_writer;
    if (m_writer.good()) {
      writer.writeToDoItem(item);
      if (writer.isStatusOk()) {
        addItem(item, DATA_LOCAL);
      }
    }
  }
}

void ZFlyEmToDoList::setRange(const ZIntCuboid &dataRange)
{
  m_dataRange = dataRange;
}

void ZFlyEmToDoList::setReady(bool ready)
{
  m_isReady = ready;
}

bool ZFlyEmToDoList::isReady() const
{
  return m_isReady;
}

#if 0
void ZFlyEmToDoList::display(
    ZPainter &painter, int slice, EDisplayStyle option,
    neutu::EAxis sliceAxis) const
{
  if (sliceAxis != getSliceAxis()) {
    return;
  }

  if (slice >= 0) {
    const int sliceRange = 5;

    int currentBlockZ = m_dvidInfo.getStartBlockIndex().getZ() - 1;

    QRect rangeRect;
    if (!m_dataRange.isEmpty()) {
      ZIntCuboid range = m_dataRange;
      range.shiftSliceAxis(getSliceAxis());

      rangeRect.setTopLeft(QPoint(range.getMinCorner().getX(),
                                  range.getMinCorner().getY()));
      rangeRect.setSize(QSize(range.getWidth(), m_dataRange.getHeight()));
    }

    if (!isReady()) {
      for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
        int z = painter.getZ(slice + ds);
        if (z >= m_dvidInfo.getStartCoordinates().getZ() ||
            z <= m_dvidInfo.getEndCoordinates().getZ()) {
          ItemSlice &itemSlice =
              const_cast<ZFlyEmToDoList&>(*this).getSlice(z, ADJUST_FULL);
          bool ready = itemSlice.isReady();

          if (!ready && m_view != NULL) {
            ready =itemSlice.isReady(
                  m_view->getViewPort(neutu::ECoordinateSystem::STACK), rangeRect);
          }
          if (!ready) {
            int blockZ = m_dvidInfo.getBlockIndexZ(z);
            if (blockZ != currentBlockZ) {
              currentBlockZ = blockZ;
              const_cast<ZFlyEmToDoList&>(*this).download(z);
            }
          }
        }
      }
    }

    for (int ds = -sliceRange; ds <= sliceRange; ++ds) {
      int z = painter.getZ(slice + ds);
      if (z < m_dvidInfo.getStartCoordinates().getSliceCoord(m_sliceAxis) ||
          z > m_dvidInfo.getEndCoordinates().getSliceCoord(m_sliceAxis)) {
        continue;
      }

      ItemSlice &itemSlice =
          const_cast<ZFlyEmToDoList&>(*this).getSlice(z, ADJUST_FULL);

      for (int i = 0; i < itemSlice.size(); ++i) {
        QMap<int, ZFlyEmToDoItem> &itemMap = itemSlice[i];
        for (QMap<int, ZFlyEmToDoItem>::const_iterator iter = itemMap.begin();
             iter != itemMap.end(); ++iter) {
          const ZFlyEmToDoItem &item = iter.value();
          item.display(painter, slice, option, sliceAxis);
        }
      }

#if 0
      const std::set<ZIntPoint>& selected = m_selector.getSelectedSet();
      for (std::set<ZIntPoint>::const_iterator iter = selected.begin();
           iter != selected.end(); ++iter) {
        ZFlyEmToDoItem &item =
            const_cast<ZFlyEmToDoList&>(*this).getItem(*iter, DATA_LOCAL);
        item.display(painter, slice, option, sliceAxis);
      }
#endif
    }
  }
}
#endif

ZFlyEmToDoItem& ZFlyEmToDoList::getItem(
    int x, int y, int z, EDataScope scope)
{
  if (scope == DATA_SYNC) {
    update(x, y, z);
  }

  if (hasLocalItem(x, y, z)) {
    int sx = x;
    int sy = y;
    int sz = z;
    zgeom::ShiftSliceAxis(sx, sy, sz, m_sliceAxis);

    return getSlice(sz).getMap(sy)[sx];
  } else {
    if (scope == DATA_LOCAL) {
      return m_emptyTodo;
    } else if (scope == DATA_GLOBAL) {
      update(x, y, z);
    }
  }

  return getItem(x, y, z, DATA_LOCAL);
}

ZFlyEmToDoItem& ZFlyEmToDoList::getItem(
    const ZIntPoint &center, EDataScope scope)
{
  return getItem(center.getX(), center.getY(), center.getZ(), scope);
}

bool ZFlyEmToDoList::toggleHitSelect()
{
  bool selecting = true;

  std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
  for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
       iter != selectedList.end(); ++iter) {
    const ZIntPoint &pt = *iter;

    if (pt == m_hitPoint) {
      selecting = false;
      break;
    }
  }

  ZFlyEmToDoItem &item = getItem(m_hitPoint, DATA_LOCAL);
  item.setSelected(selecting);
  m_selector.setSelection(m_hitPoint, selecting);

  return selecting;
}

void ZFlyEmToDoList::selectHit(bool appending)
{
  if (!appending) {
    deselectSub();
  }
  m_selector.selectObject(m_hitPoint);
  ZFlyEmToDoItem &item = getItem(m_hitPoint, DATA_LOCAL);
  if (item.isValid()) {
    item.setSelected(true);
  }
}

void ZFlyEmToDoList::deselectAll()
{
  m_selector.deselectAll();
}

void ZFlyEmToDoList::deselectSub()
{
  std::vector<ZIntPoint> selectedList = m_selector.getSelectedList();
  for (std::vector<ZIntPoint>::const_iterator iter = selectedList.begin();
       iter != selectedList.end(); ++iter) {
    const ZIntPoint &pt = *iter;
    ZFlyEmToDoItem &item = getItem(pt, DATA_LOCAL);
    if (item.isValid()) {
      item.setSelected(false);
    }
  }
  deselectAll();
}

bool ZFlyEmToDoList::hit(double x, double y, double z)
{
  const int sliceRange = 5;

  ZIntPoint hitPoint(neutu::iround(x), neutu::iround(y), neutu::iround(z));

  hitPoint.shiftSliceAxis(getSliceAxis());

  for (int slice = -sliceRange; slice <= sliceRange; ++slice) {
    int cz = hitPoint.getZ() + slice;

    ItemIterator siter(this, cz);

    while (siter.hasNext()) {
      ZFlyEmToDoItem &item = siter.next();
      if (item.hit(x, y, z)) {
        m_hitPoint = item.getPosition();
        return true;
      }
    }
  }

  return false;
}

bool ZFlyEmToDoList::hasSelected() const
{
  return !m_selector.getSelectedSet().empty();
}

std::ostream& operator<< (std::ostream &stream, const ZFlyEmToDoList &se)
{
  ZFlyEmToDoList::ItemIterator siter(&se);

  stream << "Synapses (+" << se.m_startZ << "): " << std::endl;
  while (siter.hasNext()) {
    stream << "  " << siter.next() << std::endl;
  }

  return stream;
}

std::ostream& operator<< (
    std::ostream &stream, const ZFlyEmToDoList::ItemSlice &se)
{
  stream << "Synapse slice (+" << se.m_startY << "):" << std::endl;
  for (ZFlyEmToDoList::ItemSlice::const_iterator iter = se.begin();
       iter != se.end(); ++iter) {
    const ZFlyEmToDoList::ItemMap &itemMap = *iter;
    if (!itemMap.isEmpty()) {
      for (ZFlyEmToDoList::ItemMap::const_iterator
           iter = itemMap.begin(); iter != itemMap.end(); ++iter) {
        stream << "  " << iter.value() << std::endl;
      }
    }
  }

  return stream;
}

//ZSTACKOBJECT_DEFINE_CLASS_NAME(ZFlyEmToDoList)

///////////////////Helper Classes///////////////////
QVector<ZFlyEmToDoList::ItemSlice>
ZFlyEmToDoList::ItemIterator::m_emptyZ;
QVector<ZFlyEmToDoList::ItemMap>
ZFlyEmToDoList::ItemIterator::m_emptyY;
QMap<int, ZFlyEmToDoItem> ZFlyEmToDoList::ItemIterator::m_emptyX;

ZFlyEmToDoList::ItemIterator::ItemIterator(const ZFlyEmToDoList *se) :
  m_zIterator(m_emptyZ), m_yIterator(m_emptyY), m_xIterator(m_emptyX)
{
  if (se != NULL) {
    m_zIterator = QVectorIterator<ItemSlice>(se->m_itemList);
    if (m_zIterator.hasNext()) {
      m_yIterator = QVectorIterator<ItemMap>(m_zIterator.next());
      if (m_yIterator.hasNext()) {
        m_xIterator = QMapIterator<int, ZFlyEmToDoItem>(m_yIterator.next());
      }
    }
  }
}

ZFlyEmToDoList::ItemIterator::ItemIterator(
    const ZFlyEmToDoList *se, int z) :
  m_zIterator(m_emptyZ), m_yIterator(m_emptyY), m_xIterator(m_emptyX)
{
  int index = z - se->getMinZ();
  if (index >= 0) {
    if (se != NULL) {
      if (se->m_itemList.size() > index) {
        m_yIterator = QVectorIterator<ItemMap>(se->m_itemList[index]);
        if (m_yIterator.hasNext()) {
          m_xIterator = QMapIterator<int, ZFlyEmToDoItem>(m_yIterator.next());
        }
      }
    }
  }
}

void ZFlyEmToDoList::ItemIterator::skipEmptyIterator()
{
  while (!m_xIterator.hasNext()) {
    if (m_yIterator.hasNext()) {
      m_xIterator = QMapIterator<int, ZFlyEmToDoItem>(m_yIterator.next());
    } else {
      if (m_zIterator.hasNext()) {
        m_yIterator = QVectorIterator<ItemMap>(m_zIterator.next());
      } else { //All iterators reach the end
        break;
      }
    }
  }
}

bool ZFlyEmToDoList::ItemIterator::hasNext() const
{
  const_cast<ZFlyEmToDoList::ItemIterator&>(*this).skipEmptyIterator();

  return m_xIterator.hasNext();
}

ZFlyEmToDoItem& ZFlyEmToDoList::ItemIterator::next()
{
  skipEmptyIterator();

  return const_cast<ZFlyEmToDoItem&>(m_xIterator.next().value());
}


///////////////////////////////////////////////
ZFlyEmToDoList::ItemMap::ItemMap(EDataStatus status)
{
  m_status = status;
}

///////////////////////////////////////////
ZFlyEmToDoList::ItemMap ZFlyEmToDoList::ItemSlice::m_emptyMap(
    ZFlyEmToDoList::STATUS_NULL);

ZFlyEmToDoList::ItemSlice::ItemSlice(EDataStatus status)
{
  m_startY = 0;
  m_status = status;
}

bool ZFlyEmToDoList::ItemSlice::contains(int x, int y) const
{
  const ItemMap &itemMap = getMap(y);
  if (itemMap.isValid()) {
    return itemMap.contains(x);
  }

  return false;
}

/*
bool ZFlyEmToDoList::ItemSlice::isReady(
    const QRect &rect, const QRect &range) const
{
  if (m_status == STATUS_READY) {
    return true;
  }

  if (m_status == STATUS_PARTIAL_READY) {
//    qDebug() << "Data rect: " << m_dataRect;
//    qDebug() << "New rect: " << rect;

    QRect dataRect = rect;
    if (!range.isEmpty()) {
      dataRect = rect.intersected(range);
    }

    if (!dataRect.isValid()) {
      return false;
    }

    return m_dataRect.contains(dataRect);
  }

  return false;
}
*/

void ZFlyEmToDoList::ItemSlice::setDataRect(const ZAffineRect &rect)
{
  m_dataRect = rect;
}


ZFlyEmToDoList::ItemMap& ZFlyEmToDoList::ItemSlice::getMap(int y)
{
  return const_cast<ZFlyEmToDoList::ItemMap&>(
        static_cast<const ZFlyEmToDoList::ItemSlice&>(*this).getMap(y));
}

const ZFlyEmToDoList::ItemMap& ZFlyEmToDoList::ItemSlice::getMap(int y) const
{
  if (isValid()) {
    int yIndex = y - m_startY;
    if (yIndex >= 0 && yIndex < size()) {
      return (*this)[yIndex];
    }
  }

  return m_emptyMap;
}

ZFlyEmToDoList::ItemMap&
ZFlyEmToDoList::ItemSlice::getMap(int y, EAdjustment adjust)
{
  if (!isValid()) {
    return m_emptyMap;
  }

  if (adjust == ADJUST_NONE) {
    return getMap(y);
  }

  int yIndex = y - m_startY;
  if (yIndex >= size()) {
    if (isEmpty()) {
      m_startY = y;
      resize(1);
      yIndex = 0;
    } else {
      resize(yIndex + 1);
    }
  } else if (yIndex < 0) {
    if (adjust == ADJUST_FULL) {
      ItemSlice oldSlice = *this;
      clear();
      resize(oldSlice.size() - yIndex);
      for (int i = 0; i < oldSlice.size(); ++i) {
        (*this)[i - yIndex] = oldSlice[i];
      }
      m_startY = y;
      yIndex = 0;
    }
  }

  return (*this)[yIndex];
}

void ZFlyEmToDoList::ItemSlice::addItem(
    const ZFlyEmToDoItem &item, neutu::EAxis sliceAxis)
{
  ZIntPoint center = item.getPosition();
  center.shiftSliceAxis(sliceAxis);
  ItemMap& itemMap = getMap(center.getY(), ADJUST_FULL);

  itemMap[center.getX()] = item;
}


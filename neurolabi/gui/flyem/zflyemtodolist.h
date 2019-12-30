#ifndef ZFLYEMTODOLIST_H
#define ZFLYEMTODOLIST_H

#include <QVector>
#include <QMap>
#include <QRect>

#include "geometry/zintcuboid.h"
#include "dvid/zdvidtarget.h"
#include "zstackobject.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "zselector.h"
#include "zjsonarray.h"
#include "zflyemtodoitem.h"
#include "dvid/zdvidwriter.h"

class ZStackView;

class ZFlyEmToDoList : public ZStackObject
{
public:
  ZFlyEmToDoList();
  virtual ~ZFlyEmToDoList();

  enum EDataScope {
    DATA_GLOBAL, DATA_LOCAL, DATA_SYNC
  };

  enum EDataStatus {
    STATUS_NORMAL, STATUS_NULL, STATUS_PARTIAL_READY, STATUS_READY
  };

  enum EAdjustment {
    ADJUST_NONE, ADJUST_EXTEND, ADJUST_FULL
  };

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_TODO_LIST;
  }

  void setDvidTarget(const ZDvidTarget &target);
  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void setDvidInfo(const ZDvidInfo &info);

  class ItemMap : public QMap<int, ZFlyEmToDoItem> {
  public:
    ItemMap(EDataStatus status = STATUS_NORMAL);
    bool isValid() const { return m_status != STATUS_NULL; }

  private:
    EDataStatus m_status;
  };

  class ItemSlice : public QVector<ItemMap> {
  public:
    ItemSlice(EDataStatus status = STATUS_NORMAL);

    void addItem(const ZFlyEmToDoItem &item, neutu::EAxis sliceAxis);
    const ItemMap& getMap(int y) const;
    ItemMap& getMap(int y);
    ItemMap& getMap(int y, EAdjustment adjust);

    bool isValid() const { return m_status != STATUS_NULL; }
    bool isReady() const { return m_status == STATUS_READY; }
    bool isReady(const QRect &rect, const QRect &range) const;

    void setStatus(EDataStatus status) {
      m_status = status;
    }

    void setDataRect(const QRect &rect);

    bool contains(int x, int y) const;

    friend std::ostream& operator<< (std::ostream &stream, const ItemSlice &se);

  private:
    int m_startY;
    EDataStatus m_status;
    QRect m_dataRect;
    static ItemMap m_emptyMap;
  };

  void setRange(const ZIntCuboid &dataRange);
  void setReady(bool ready);
  bool isReady() const;

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

  bool removeItem(const ZIntPoint &pt, EDataScope scope);
  bool removeItem(int x, int y, int z, EDataScope scope);

  void addItem(const ZFlyEmToDoItem &item, EDataScope scope);

  ZFlyEmToDoItem &getItem(int x, int y, int z, EDataScope scope);
  ZFlyEmToDoItem &getItem(const ZIntPoint &center, EDataScope scope);

  ItemMap& getItemMap(int y, int z);
  const ItemMap &getItemMap(int y, int z) const;
  ItemMap& getItemMap(int y, int z, EAdjustment adjust);


  const ItemSlice& getSlice(int z) const;
  ItemSlice& getSlice(int z);
  ItemSlice& getSlice(int z, EAdjustment adjust);

  int getMinZ() const;
  int getMaxZ() const;

  bool hasLocalItem(int x, int y, int z) const;

  bool toggleHitSelect();
  void selectHit(bool appending);
  void deselectAll();

  void deselect(bool recursive);

//  const std::string& className() const;

  bool hit(double x, double y, double z);

//  void downloadForLabel(uint64_t label);
  void download(int z);

  bool hasSelected() const;
  const ZSelector<ZIntPoint>& getSelector() const { return m_selector; }
  ZSelector<ZIntPoint>& getSelector() { return m_selector; }

  void clearCache();

  ZIntCuboid update(const ZIntCuboid &box);
  void update(int x, int y, int z);
  void update(const ZIntPoint &pt);
  void update(const std::vector<ZIntPoint> &ptArray);

  void updatePartner(ZFlyEmToDoItem &item);

  void attachView(ZStackView *view);

  friend std::ostream& operator<< (
      std::ostream &stream, const ZFlyEmToDoList &se);

  class ItemIterator {
  public:
    ItemIterator(const ZFlyEmToDoList *se);
    ItemIterator(const ZFlyEmToDoList *se, int z);

    bool hasNext() const;
    ZFlyEmToDoItem& next(); //out-of-range query leads to undefined behavior

  private:
    void skipEmptyIterator();

  private:
    static QVector<ItemSlice> m_emptyZ;
    static QVector<ItemMap> m_emptyY;
    static QMap<int, ZFlyEmToDoItem> m_emptyX;

    QVectorIterator<ItemSlice> m_zIterator;
    QVectorIterator<ItemMap> m_yIterator;
    QMapIterator<int, ZFlyEmToDoItem> m_xIterator;
  };

private:
  void init();
  void updateFromCache(int z);
  void deselectSub();

private:
  QVector<ItemSlice> m_itemList;
//  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;
  static ZFlyEmToDoItem m_emptyTodo;
  static ItemSlice m_emptySlice;

  int m_startZ;
//  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidWriter m_writer;
  ZDvidInfo m_dvidInfo;

  ZSelector<ZIntPoint> m_selector;

  ZStackView *m_view;
  int m_maxPartialArea;
  bool m_fetchingFullAllowed = false;

  ZIntCuboid m_dataRange;
  bool m_isReady;
};

#endif // ZFLYEMTODOLIST_H

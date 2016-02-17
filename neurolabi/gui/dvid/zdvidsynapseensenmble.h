#ifndef ZDVIDSYNAPSEENSENMBLE_H
#define ZDVIDSYNAPSEENSENMBLE_H

#include <QMap>
#include <QVector>
#include <QCache>

#include <iostream>

#include "zdvidtarget.h"
#include "zstackobject.h"
#include "zdvidreader.h"
#include "zdvidinfo.h"
#include "zdvidsynapse.h"
#include "zselector.h"
#include "zjsonarray.h"

class ZStackView;
class ZIntCuboid;

class ZDvidSynapseEnsemble : public ZStackObject
{
public:
  ZDvidSynapseEnsemble();
  virtual ~ZDvidSynapseEnsemble() {}

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
    return ZStackObject::TYPE_DVID_SYNAPE_ENSEMBLE;
  }

  void setDvidTarget(const ZDvidTarget &target);

  class SynapseMap : public QMap<int, ZDvidSynapse> {
  public:
    SynapseMap(EDataStatus status = STATUS_NORMAL);
    bool isValid() const { return m_status != STATUS_NULL; }

  private:
    EDataStatus m_status;
  };

  class SynapseSlice : public QVector<SynapseMap> {
  public:
    SynapseSlice(EDataStatus status = STATUS_NORMAL);

    void addSynapse(const ZDvidSynapse &synapse, NeuTube::EAxis sliceAxis);
    const SynapseMap& getMap(int y) const;
    SynapseMap& getMap(int y);
    SynapseMap& getMap(int y, EAdjustment adjust);

    bool isValid() const { return m_status != STATUS_NULL; }
    bool isReady() const { return m_status == STATUS_READY; }
    bool isReady(const QRect &rect, const QRect &range) const;

    void setStatus(EDataStatus status) {
      m_status = status;
    }

    void setDataRect(const QRect &rect);

    bool contains(int x, int y) const;

    friend std::ostream& operator<< (
        std::ostream &stream, const SynapseSlice &se);

  private:
    int m_startY;
    EDataStatus m_status;
    QRect m_dataRect;
    static SynapseMap m_emptyMap;
  };

  void setRange(const ZIntCuboid &dataRange);

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  bool removeSynapse(const ZIntPoint &pt, EDataScope scope);
  bool removeSynapse(int x, int y, int z, EDataScope scope);

  /*!
   * \brief Add a synapse
   *
   * Adding a synapse to the ensemble. If \a synapse is selected, the selection
   * will be preserved. It will also overwrite the existing synapse at the same
   * location, but old selections will always be preserved.
   *
   * \param synapse synapse to add
   * \param scope Operation scope
   */
  void addSynapse(const ZDvidSynapse &synapse, EDataScope scope);
//  void commitSynapse(const ZIntPoint &pt);
  void moveSynapse(const ZIntPoint &from, const ZIntPoint &to,
                   EDataScope scope);

  void removeSynapseLink(const ZIntPoint &v1, const ZIntPoint &v2);

  ZDvidSynapse &getSynapse(int x, int y, int z, EDataScope scope);
  ZDvidSynapse &getSynapse(const ZIntPoint &center, EDataScope scope);

  SynapseMap& getSynapseMap(int y, int z);
  const SynapseMap &getSynapseMap(int y, int z) const;
  SynapseMap& getSynapseMap(int y, int z, EAdjustment adjust);


  const SynapseSlice& getSlice(int z) const;
  SynapseSlice& getSlice(int z);
  SynapseSlice& getSlice(int z, EAdjustment adjust);

//  NeuTube::EAxis getSliceAxis() const { return m_sliceAxis; }
//  void setSliceAxis(NeuTube::EAxis axis) { m_sliceAxis = axis; }

//  bool deleteSynapse(int x, int y, int z);

  int getMinZ() const;
  int getMaxZ() const;

  bool hasLocalSynapse(int x, int y, int z) const;

  bool toggleHitSelect();
  void selectHit(bool appending);
  void selectHitWithPartner(bool appending);
  void toggleHitSelectWithPartner();

  const std::string& className() const;

  bool hit(double x, double y, double z);

  void downloadForLabel(uint64_t label);
  void download(int z);

  bool hasSelected() const;
  const ZSelector<ZIntPoint>& getSelector() const { return m_selector; }
  ZSelector<ZIntPoint>& getSelector() { return m_selector; }

  ZIntCuboid update(const ZIntCuboid &box);
  void update(int x, int y, int z);
  void update(const ZIntPoint &pt);

  void updatePartner(ZDvidSynapse &synapse);

  void attachView(ZStackView *view);

  friend std::ostream& operator<< (
      std::ostream &stream, const ZDvidSynapseEnsemble &se);

  class SynapseIterator {
  public:
    SynapseIterator(const ZDvidSynapseEnsemble *se);
    SynapseIterator(const ZDvidSynapseEnsemble *se, int z);

    bool hasNext() const;
    ZDvidSynapse& next(); //out-of-range query leads to undefined behavior

  private:
    void skipEmptyIterator();

  private:
    static QVector<SynapseSlice> m_emptyZ;
    static QVector<SynapseMap> m_emptyY;
    static QMap<int, ZDvidSynapse> m_emptyX;

    QVectorIterator<SynapseSlice> m_zIterator;
    QVectorIterator<SynapseMap> m_yIterator;
    QMapIterator<int, ZDvidSynapse> m_xIterator;
  };

private:
  void init();
  void updateFromCache(int z);

private:
  QVector<SynapseSlice> m_synapseEnsemble;
//  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;
  static ZDvidSynapse m_emptySynapse;
  static SynapseSlice m_emptySlice;

  int m_startZ;
//  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;

  ZSelector<ZIntPoint> m_selector;

  ZStackView *m_view;
  int m_maxPartialArea;

//  NeuTube::EAxis m_sliceAxis;

  ZIntCuboid m_dataRange;

  mutable QCache<int, SynapseSlice> m_sliceCache;
};

#endif // ZDVIDSYNAPSEENSENMBLE_H

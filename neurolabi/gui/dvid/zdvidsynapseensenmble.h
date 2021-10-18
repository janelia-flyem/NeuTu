#ifndef ZDVIDSYNAPSEENSENMBLE_H
#define ZDVIDSYNAPSEENSENMBLE_H

#include <QMap>
#include <QVector>
#include <QCache>
#include <QMutex>
#include <QVector>

#include <iostream>

#include "zdvidtarget.h"
#include "zstackobject.h"
#include "zdvidinfo.h"
#include "zdvidsynapse.h"
#include "zselector.h"
#include "zjsonarray.h"
#include "zdvidwriter.h"
#include "zresolution.h"
#include "geometry/zintcuboid.h"

class ZStackView;
class ZIntCuboid;
class ZFlyEmSynapseDataFetcher;

class ZDvidSynapseEnsemble : public ZStackObject
{
public:
  ZDvidSynapseEnsemble();
  virtual ~ZDvidSynapseEnsemble() {}

  enum class EDataScope {
    GLOBAL, LOCAL, SYNC
  };

  enum class EDataStatus {
    NORMAL, NONE, PARTIAL_READY, READY
  };

  enum EAdjustment {
    ADJUST_NONE, ADJUST_EXTEND, ADJUST_FULL
  };

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_SYNAPE_ENSEMBLE;
  }

  void setDvidTarget(const ZDvidTarget &target);
  void setDvidInfo(const ZDvidInfo &info);

  ZResolution getResolution() const {
    return m_resolution;
  }
  void setResolution(const ZResolution &resolution) {
    m_resolution = resolution;
  }

  class SynapseMap : public QMap<int, ZDvidSynapse> {
  public:
    SynapseMap(EDataStatus status = EDataStatus::NORMAL);
    bool isValid() const { return m_status != EDataStatus::NONE; }
    void updateRadius();

  private:
    EDataStatus m_status;
  };

  class SynapseSlice : public QVector<SynapseMap> {
  public:
    SynapseSlice(EDataStatus status = EDataStatus::NORMAL);

    void addSynapse(const ZDvidSynapse &synapse, neutu::EAxis sliceAxis);
    const SynapseMap& getMap(int y) const;
    SynapseMap& getMap(int y);
    SynapseMap& getMap(int y, EAdjustment adjust);

    bool isValid() const { return m_status != EDataStatus::NONE; }
    bool isReady() const { return m_status == EDataStatus::READY; }
    bool isReady(const QRect &rect) const;
    bool isReady(const QRect &rect, const QRect &range) const;

    void setStatus(EDataStatus status) {
      m_status = status;
    }

    void setDataRect(const QRect &rect);

    bool contains(int x, int y) const;
    void updateRadius();

    friend std::ostream& operator<< (
        std::ostream &stream, const SynapseSlice &se);

  private:
    int m_startY;
    EDataStatus m_status;
    QRect m_dataRect;
    static SynapseMap m_emptyMap;
  };

  void setRange(const ZIntCuboid &dataRange);

  bool display(
      QPainter */*painter*/, const DisplayConfig &/*config*/) const {
    return false;
  }

#if 0
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;
#endif

  void clearCache();

  bool removeSynapse(const ZIntPoint &pt, EDataScope scope);
  bool removeSynapse(int x, int y, int z, EDataScope scope);

  bool removeSynapseUnsync(const ZIntPoint &pt, EDataScope scope);
  bool removeSynapseUnsync(int x, int y, int z, EDataScope scope);

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
  void addSynapseUnsync(const ZDvidSynapse &synapse, EDataScope scope);
  void addSynapse(const ZDvidSynapse &synapse, EDataScope scope);

//  void commitSynapse(const ZIntPoint &pt);
  void moveSynapseUnsync(const ZIntPoint &from, const ZIntPoint &to, EDataScope scope);
  void moveSynapse(const ZIntPoint &from, const ZIntPoint &to,
                   EDataScope scope);

  void annotateSynapseUnsync(int x, int y, int z, const ZJsonObject &propJson,
                       EDataScope scope);
  void annotateSynapseUnsync(const ZIntPoint &pt, const ZJsonObject &propJson,
                       EDataScope scope);
  void annotateSynapse(int x, int y, int z, const ZJsonObject &propJson,
                       EDataScope scope);
  void annotateSynapse(const ZIntPoint &pt, const ZJsonObject &propJson,
                       EDataScope scope);

  void setUserNameUnsync(const ZIntPoint &pt, const std::string &userName,
                   EDataScope scope);
  void setUserNameUnsync(int x, int y, int z, const std::string &userName,
                   EDataScope scope);

  void setUserName(const ZIntPoint &pt, const std::string &userName,
                   EDataScope scope);
  void setUserName(int x, int y, int z, const std::string &userName,
                   EDataScope scope);

  void setConfidenceUnsync(int x, int y, int z, const double c,
                     EDataScope scope);
  void setConfidenceUnsync(const ZIntPoint &pt, const double c,
                     EDataScope scope);

  void setConfidence(int x, int y, int z, const double c,
                     EDataScope scope);
  void setConfidence(const ZIntPoint &pt, const double c,
                     EDataScope scope);

  void removeSynapseLink(const ZIntPoint &v1, const ZIntPoint &v2);

  ZDvidSynapse &getSynapseUnsync(int x, int y, int z, EDataScope scope);
  ZDvidSynapse &getSynapseUnsync(const ZIntPoint &center, EDataScope scope);

  ZDvidSynapse &getSynapse(int x, int y, int z, EDataScope scope);
  ZDvidSynapse &getSynapse(const ZIntPoint &center, EDataScope scope);

  SynapseMap& getSynapseMapUnsync(int y, int z);
  const SynapseMap &getSynapseMapUnsync(int y, int z) const;
  SynapseMap& getSynapseMapUnsync(int y, int z, EAdjustment adjust);


  const SynapseSlice& getSliceUnsync(int z) const;
  SynapseSlice& getSliceUnsync(int z);
  SynapseSlice& getSliceUnsync(int z, EAdjustment adjust);

  void setReadyUnsync(const ZIntCuboid &box);
  void setReady(const ZIntCuboid &box);

  void setReady(bool ready);
  bool isReady() const;

  int getMinZUnsync() const;
  int getMaxZUnsync() const;

  bool hasLocalSynapseUnsync(int x, int y, int z) const;

  bool toggleHitSelectUnsync();
  void selectHitUnsync(bool appending);
  void selectHitWithPartnerUnsync(bool appending);
  void toggleHitSelectWithPartnerUnsync();

  void selectHitWithPartner(bool appending);
  void toggleHitSelectWithPartner();

  void selectElementUnsync(const ZIntPoint &pt, bool appending);
  void selectWithPartnerUnsync(const ZIntPoint &pt, bool appending);

  void selectWithPartner(const ZIntPoint &pt, bool appending);

  void deselectUnsync(bool recursive);

//  const std::string& className() const;

  bool hit(double x, double y, double z);
  bool hit(const ZIntPoint &pt);

  void downloadForLabelUnsync(uint64_t label);
  void downloadUnsync(int z);
  void download(int z);
  void downloadUnsync(const QVector<int> &zs);
  void setDataFetcher(ZFlyEmSynapseDataFetcher *fetcher);

  bool hasSelected() const;
  const ZSelector<ZIntPoint>& getSelector() const { return m_selector; }
  ZSelector<ZIntPoint>& getSelector() { return m_selector; }

  ZIntCuboid updateUnsync(const ZIntCuboid &box);
  ZIntCuboid update(const ZIntCuboid &box);

  void updateUnsync(int x, int y, int z);
  void updateUnsync(const ZIntPoint &pt);

  void update(int x, int y, int z);
  void update(const ZIntPoint &pt);

  void updatePartnerUnsync(const ZIntPoint &pt);
  void updatePartner(const ZIntPoint &pt);

  void updatePartner(ZDvidSynapse &synapse);

  void updateRadiusUnsync();
  void updateRadius();

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

    QMutex m_dataMutex;
  };

private:
  void init();
  void updateFromCacheUnsync(int z);
  void deselectSubUnsync();
  void unsyncedFetch(const ZIntCuboid &box);
  void syncedFetch(const ZIntCuboid &box, int startZ, int endZ, bool isFull);

private:
  QVector<SynapseSlice> m_synapseEnsemble;
//  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;
  static ZDvidSynapse m_emptySynapse;
  static SynapseSlice m_emptySlice;

  int m_startZ = 0;
//  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidWriter m_writer;
  ZDvidInfo m_dvidInfo;

  ZSelector<ZIntPoint> m_selector;

  ZStackView *m_view = nullptr;
  int m_maxPartialArea;
  bool m_fetchingFullAllowed = false;
//  int m_maxFetchingArea = 4153344; //2048*2048 by default. 0 means no restriction.

//  NeuTube::EAxis m_sliceAxis;

  bool m_isReady = false;

  ZIntCuboid m_dataRange;
  ZResolution m_resolution;

  ZFlyEmSynapseDataFetcher *m_dataFetcher = nullptr;

  mutable QMutex m_dataMutex;
  mutable QCache<int, SynapseSlice> m_sliceCache;
};

#endif // ZDVIDSYNAPSEENSENMBLE_H

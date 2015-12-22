#ifndef ZDVIDSYNAPSEENSENMBLE_H
#define ZDVIDSYNAPSEENSENMBLE_H

#include <QMap>
#include <QVector>

#include <iostream>

#include "zdvidtarget.h"
#include "zstackobject.h"
#include "zdvidreader.h"
#include "zdvidinfo.h"
#include "zdvidsynapse.h"
#include "zselector.h"

class ZDvidSynapseEnsemble : public ZStackObject
{
public:
  ZDvidSynapseEnsemble();
  virtual ~ZDvidSynapseEnsemble() {}

  enum EDataScope {
    DATA_GLOBAL, DATA_LOCAL, DATA_SYNC
  };

  void setDvidTarget(const ZDvidTarget &target);

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  bool removeSynapse(const ZIntPoint &pt, EDataScope scope);
  bool removeSynapse(int x, int y, int z, EDataScope scope);

  void addSynapse(const ZDvidSynapse &synapse, EDataScope scope);
//  void commitSynapse(const ZIntPoint &pt);
  void moveSynapse(const ZIntPoint &from, const ZIntPoint &to);

  ZDvidSynapse &getSynapse(int x, int y, int z, EDataScope scope);
  ZDvidSynapse &getSynapse(const ZIntPoint &center, EDataScope scope);

  QMap<int, ZDvidSynapse>& getSynapseMap(int y, int z);
  QVector<QMap<int, ZDvidSynapse> >& getSlice(int z);

//  bool deleteSynapse(int x, int y, int z);

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
    QVector<QVector<QMap<int, ZDvidSynapse> > > m_emptyZ;
    QVector<QMap<int, ZDvidSynapse> > m_emptyY;
    QMap<int, ZDvidSynapse> m_emptyX;

    QVectorIterator<QVector<QMap<int, ZDvidSynapse> > > m_zIterator;
    QVectorIterator<QMap<int, ZDvidSynapse> > m_yIterator;
    QMapIterator<int, ZDvidSynapse> m_xIterator;
  };

private:
  void update(int x, int y, int z);
  void update(const ZIntPoint &pt);
  void updatePartner(ZDvidSynapse &synapse);

private:
  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;
  ZDvidSynapse m_emptySynapse;

  int m_startZ;
  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;

  ZSelector<ZIntPoint> m_selector;
};

#endif // ZDVIDSYNAPSEENSENMBLE_H

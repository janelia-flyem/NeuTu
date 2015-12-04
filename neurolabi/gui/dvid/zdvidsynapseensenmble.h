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

  void setDvidTarget(const ZDvidTarget &target);

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  void removeSynapse(int x, int y, int z);

  void addSynapse(const ZDvidSynapse &synapse);

  ZDvidSynapse &getSynapse(int x, int y, int z);
  ZDvidSynapse &getSynapse(const ZIntPoint &center);

  QMap<int, ZDvidSynapse>& getSynapseMap(int y, int z);
  QVector<QMap<int, ZDvidSynapse> >& getSlice(int z);

  void selectHit(bool appending);

  const std::string& className() const;

  bool hit(double x, double y, double z);

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
  void download(int z);

private:
  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;

  int m_startZ;
  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;

  ZSelector<ZIntPoint> m_selector;
};

#endif // ZDVIDSYNAPSEENSENMBLE_H

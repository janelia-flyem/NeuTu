#ifndef ZDVIDSYNAPSEENSENMBLE_H
#define ZDVIDSYNAPSEENSENMBLE_H

#include <QMap>
#include <QVector>

#include "zdvidtarget.h"
#include "zstackobject.h"
#include "zdvidreader.h"
#include "zdvidinfo.h"
#include "zdvidsynapse.h"

class ZDvidSynapseEnsenmble : public ZStackObject
{
public:
  ZDvidSynapseEnsenmble();
  virtual ~ZDvidSynapseEnsenmble() {}

  void setDvidTarget(const ZDvidTarget &target);

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  void removeSynapse(int x, int y, int z);

  QMap<int, ZDvidSynapse>& getSynapseMap(int y, int z);
  QVector<QMap<int, ZDvidSynapse> >& getSlice(int z);

private:
  void download(int z);

private:
  QVector<QVector<QMap<int, ZDvidSynapse> > > m_synapseEnsemble;

  int m_startZ;
  int m_startY;
  ZDvidTarget m_dvidTarget;
  ZDvidReader m_reader;
  ZDvidInfo m_dvidInfo;
};

#endif // ZDVIDSYNAPSEENSENMBLE_H

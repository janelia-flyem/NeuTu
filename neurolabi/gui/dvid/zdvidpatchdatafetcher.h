#ifndef ZDVIDPATCHDATAFETCHER_H
#define ZDVIDPATCHDATAFETCHER_H

#include <QObject>
#include <QMutex>

#include "zimage.h"
#include "zrect2d.h"
#include "zdviddatafetcher.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "zthreadfuturemap.h"

class ZDvidTileEnsemble;

class ZDvidPatchDataFetcher : public ZDvidDataFetcher
{
  Q_OBJECT

public:
  explicit ZDvidPatchDataFetcher(QObject *parent = 0);
  ~ZDvidPatchDataFetcher();

  bool updatePatch(ZDvidTileEnsemble *slice, int z);
  bool updatePatch(ZDvidTileEnsemble *slice);

signals:
  void dataFetched(ZDvidPatchDataFetcher*);

public slots:

private:
  void fetch(const ZIntCuboid &region);
  void init();

private:
  ZImage *m_patch;
};

#endif // ZDVIDPATCHDATAFETCHER_H

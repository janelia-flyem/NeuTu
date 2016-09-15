#ifndef ZDVIDPATCHDATAUPDATER_H
#define ZDVIDPATCHDATAUPDATER_H

#include <QObject>

#include "zsharedpointer.h"

class ZDvidTileEnsemble;
class ZDvidPatchDataFetcher;
class ZStackDoc;

class ZDvidPatchDataUpdater : public QObject
{
  Q_OBJECT
public:
  explicit ZDvidPatchDataUpdater(QObject *parent = 0);

  void setData(ZDvidTileEnsemble *se, ZSharedPointer<ZStackDoc> doc);

signals:

public slots:
  void updateData(ZDvidPatchDataFetcher *fetcher);

private:
  ZDvidTileEnsemble *m_se;
  ZSharedPointer<ZStackDoc> m_doc;
};

#endif // ZDVIDPATCHDATAUPDATER_H

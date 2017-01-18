#ifndef ZFLYEMSYNAPSEDATAUPDATER_H
#define ZFLYEMSYNAPSEDATAUPDATER_H

#include <QObject>

#include "zsharedpointer.h"

class ZDvidSynapseEnsemble;
class ZFlyEmSynapseDataFetcher;
class ZStackDoc;

class ZFlyEmSynapseDataUpdater : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmSynapseDataUpdater(QObject *parent = 0);

  void setData(ZDvidSynapseEnsemble *se, ZSharedPointer<ZStackDoc> doc);

signals:

public slots:
  void updateData(ZFlyEmSynapseDataFetcher* fetcher);

private:
  ZDvidSynapseEnsemble *m_se;
  ZSharedPointer<ZStackDoc> m_doc;
};

#endif // ZFLYEMSYNAPSEDATAUPDATER_H

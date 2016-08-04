#ifndef ZFLYEMSYNAPSEDATAFETCHER_H
#define ZFLYEMSYNAPSEDATAFETCHER_H

#include <QObject>
#include <QMutex>

#include "zintcuboid.h"
#include "zjsonarray.h"

class ZDvidSynapseEnsemble;

class ZFlyEmSynapseDataFetcher : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmSynapseDataFetcher(QObject *parent = 0);

  void setRegion(const ZIntCuboid &box);
  void resetRegion();
  ZIntCuboid takeRegion();

  void fetch();
  void addSynapse(ZDvidSynapseEnsemble *se);

signals:
  void dataFetched();

public slots:

private:
  ZIntCuboid m_dataRegion;
  QMutex m_regionMutex;
  QMutex m_dataMutex;

  ZJsonArray m_data;
};

#endif // ZFLYEMSYNAPSEDATAFETCHER_H

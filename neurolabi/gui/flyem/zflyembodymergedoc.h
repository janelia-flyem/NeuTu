#ifndef ZFLYEMBODYMERGEDOC_H
#define ZFLYEMBODYMERGEDOC_H

#include "zstackdoc.h"
#include <QList>

#include "flyem/zflyembodymerger.h"

class ZArray;

class ZFlyEmBodyMergeDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyMergeDoc(ZStack *stack, QObject *parent = 0);

public:
  void updateBodyObject();

private:
  QList<ZObject3dScan*> extractAllObject();

signals:

public slots:

private:
  ZArray *m_originalLabel;
  ZFlyEmBodyMerger m_bodyMerger;
};

#endif // ZFLYEMBODYMERGEDOC_H

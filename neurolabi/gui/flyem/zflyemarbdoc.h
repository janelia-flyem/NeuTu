#ifndef ZFLYEMARBDOC_H
#define ZFLYEMARBDOC_H

#include "flyem/zflyemproofdoc.h"

class ZFlyEmArbDoc : public ZFlyEmProofDoc
{
  Q_OBJECT

public:
  explicit ZFlyEmArbDoc(QObject *parent = 0);

  void setDvidTarget(const ZDvidTarget &target);
  void prepareDvidData();

private:
  void initArbGraySlice();
};

#endif // ZFLYEMARBDOC_H

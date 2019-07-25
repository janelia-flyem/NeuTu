#ifndef ZFLYEMARBDOC_H
#define ZFLYEMARBDOC_H

#include "flyem/zflyemproofdoc.h"

class ZFlyEmArbDoc : public ZFlyEmProofDoc
{
  Q_OBJECT

public:
  explicit ZFlyEmArbDoc(QObject *parent = 0);

  bool setDvid(const ZDvidEnv &env) override;
//  void setDvidTarget(const ZDvidTarget &target);
  void prepareDvidData(const ZDvidEnv &env) override;

private:
  void initArbGraySlice();
};

#endif // ZFLYEMARBDOC_H

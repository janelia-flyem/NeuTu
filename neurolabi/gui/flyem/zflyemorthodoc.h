#ifndef ZFLYEMORTHODOC_H
#define ZFLYEMORTHODOC_H

#include "flyem/zflyemproofdoc.h"

class ZFlyEmOrthoDoc : public ZFlyEmProofDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoDoc(QObject *parent = 0);

  void updateStack(const ZIntPoint &center);
  void prepareDvidData();

  ZDvidSynapseEnsemble* getDvidSynapseEnsemble(NeuTube::EAxis axis) const;

signals:

public slots:

private:
  void init();
  void initSynapseEnsemble();
  void initSynapseEnsemble(NeuTube::EAxis axis);

private:
  int m_width;
  int m_height;
  int m_depth;
};

#endif // ZFLYEMORTHODOC_H

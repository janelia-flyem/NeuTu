#ifndef ZFLYEMORTHODOC_H
#define ZFLYEMORTHODOC_H

#include "flyem/zflyemproofdoc.h"

class ZCrossHair;

class ZFlyEmOrthoDoc : public ZFlyEmProofDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoDoc(QObject *parent = 0);

  void updateStack(const ZIntPoint &center);
  void prepareDvidData();

  ZDvidSynapseEnsemble* getDvidSynapseEnsemble(neutube::EAxis axis) const;

  ZCrossHair* getCrossHair() const;

  void setCrossHairCenter(double x, double y, neutube::EAxis axis);

signals:

public slots:

private:
  void init();
  void initSynapseEnsemble();
  void initSynapseEnsemble(neutube::EAxis axis);
  void initTodoList();
  void initTodoList(neutube::EAxis axis);

private:
  int m_width;
  int m_height;
  int m_depth;
};

#endif // ZFLYEMORTHODOC_H

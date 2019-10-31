#ifndef ZFLYEMORTHODOC_H
#define ZFLYEMORTHODOC_H

#include "flyem/zflyemproofdoc.h"

class ZCrossHair;
class ZIntPoint;
class ZDvidEnv;

class ZFlyEmOrthoDoc : public ZFlyEmProofDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoDoc(QObject *parent = 0);
  explicit ZFlyEmOrthoDoc(int width, int height, int depth, QObject *parent = 0);

  void updateStack(const ZIntPoint &center);
  void prepareDvidData(const ZDvidEnv &env);

  ZDvidSynapseEnsemble* getDvidSynapseEnsemble(neutu::EAxis axis) const;

  ZCrossHair* getCrossHair() const;
  ZPoint getCrossHairCenter();

  void setCrossHairCenter(double x, double y, neutu::EAxis axis);
  void setCrossHairCenter(const ZIntPoint &center);

  void setSize(int width, int height, int depth);

signals:

public slots:

private:
  void init(int width, int height, int depth);
  void initSynapseEnsemble();
  void initSynapseEnsemble(neutu::EAxis axis);
  void initTodoList();
  void initTodoList(neutu::EAxis axis);

private:
  int m_width;
  int m_height;
  int m_depth;
};

#endif // ZFLYEMORTHODOC_H

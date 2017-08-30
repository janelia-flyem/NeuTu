#ifndef Z3DSCENE_H
#define Z3DSCENE_H

#include <QGraphicsScene>

class Z3DNetworkEvaluator;

class Z3DScene : public QGraphicsScene
{
  Q_OBJECT
public:
  explicit Z3DScene(int width, int height, bool stereo, QObject* parent = nullptr);

  virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

  void setNetworkEvaluator(Z3DNetworkEvaluator* n) { m_networkEvaluator = n; }
  void setFakeStereoOnce() { m_fakeStereoOnce = true; }

private:
  Z3DNetworkEvaluator* m_networkEvaluator;
  bool m_isStereoScene;
  bool m_fakeStereoOnce;
};

#endif // Z3DSCENE_H

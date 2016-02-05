#ifndef ZFLYEMORTHOWIDGET_H
#define ZFLYEMORTHOWIDGET_H

#include <QWidget>

class ZDvidTarget;
class ZFlyEmOrthoMvc;
class ZFlyEmOrthoDoc;
class FlyEmOrthoControlForm;
class ZIntPoint;

class ZFlyEmOrthoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWidget(const ZDvidTarget &target, QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;

signals:

public slots:
  void moveUp();
  void moveDown();
  void moveLeft();
  void moveRight();

  void moveTo(const ZIntPoint &center);

private:
  void init(const ZDvidTarget &target);
  void connectSignalSlot();

private:
  ZFlyEmOrthoMvc *m_xyMvc;
  ZFlyEmOrthoMvc *m_yzMvc;
  ZFlyEmOrthoMvc *m_xzMvc;
  FlyEmOrthoControlForm *m_controlForm;
};

#endif // ZFLYEMORTHOWIDGET_H

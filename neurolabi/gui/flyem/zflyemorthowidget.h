#ifndef ZFLYEMORTHOWIDGET_H
#define ZFLYEMORTHOWIDGET_H

#include <QWidget>

class ZDvidTarget;
class ZFlyEmOrthoMvc;
class ZFlyEmOrthoDoc;
class FlyEmOrthoControlForm;
class ZIntPoint;
class ZStackView;
class ZStackMvc;
class ZWidgetMessage;

class ZFlyEmOrthoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWidget(const ZDvidTarget &target, QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;

signals:
  void bookmarkEdited(int, int, int);
  void synapseEdited(int, int, int);
  void zoomingTo(int, int, int);
  void bodyMergeEdited();

public slots:
  void moveUp();
  void moveDown();
  void moveLeft();
  void moveRight();

  void moveTo(double x, double y, double z);
  void moveTo(const ZIntPoint &center);
  void syncView();
  void syncViewWith(ZFlyEmOrthoMvc *mvc);
  void locateMainWindow();
  void syncMergeWithDvid();
  void processMessage(const ZWidgetMessage &message);

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

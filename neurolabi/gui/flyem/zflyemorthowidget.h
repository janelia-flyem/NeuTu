#ifndef ZFLYEMORTHOWIDGET_H
#define ZFLYEMORTHOWIDGET_H

#include <QWidget>
#include <QVector>

class ZDvidTarget;
class ZFlyEmOrthoMvc;
class ZFlyEmOrthoDoc;
class FlyEmOrthoControlForm;
class ZIntPoint;
class ZStackView;
class ZStackMvc;
class ZWidgetMessage;
class ZDvidEnv;

class ZFlyEmOrthoWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWidget(const ZDvidEnv &env, QWidget *parent = 0);
  explicit ZFlyEmOrthoWidget(const ZDvidEnv &env, int width, int height, int depth,
      QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;

signals:
  void bookmarkEdited(int, int, int);
  void synapseEdited(int, int, int);
  void synapseVerified(int x, int y, int z, bool verified);
  void todoEdited(int, int, int);
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
  void syncCrossHair();
  void syncCrossHairWith(ZFlyEmOrthoMvc *mvc);
  void syncImageScreen();
  void syncImageScreenWith(ZFlyEmOrthoMvc *mvc);
  void syncHighlightModeWith(ZFlyEmOrthoMvc *mvc);
  void syncHighlightMode();
  void locateMainWindow();
  void resetCrosshair();
  void reloadStack();
  void syncMergeWithDvid();
  void processMessage(const ZWidgetMessage &message);
  void setSegmentationVisible(bool on);
  void setDataVisible(bool on);
  void setHighContrast(bool on);
  void setSmoothDisplay(bool on);
  void toggleSegmentation();
  void toggleData();
  void updateImageScreen();
  void notifyBodyMergeEdited();
  void showCrosshair(bool on);
  void toggleAutoReload(bool on);

public:
  void keyPressEvent(QKeyEvent *event);

private:
//  void init(const ZDvidTarget &target);
  void init(const ZDvidEnv &env, int width, int height, int depth);
  void connectSignalSlot();
  void beginViewSync();
  void endViewSync();
  void beginCrossHairSync();
  void endCrossHairSync();

private:
  ZFlyEmOrthoMvc *m_xyMvc;
  ZFlyEmOrthoMvc *m_yzMvc;
  ZFlyEmOrthoMvc *m_xzMvc;
  QVector<ZFlyEmOrthoMvc*> m_mvcArray;
  FlyEmOrthoControlForm *m_controlForm;
};

#endif // ZFLYEMORTHOWIDGET_H

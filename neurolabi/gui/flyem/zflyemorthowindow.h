#ifndef ZFLYEMORTHOWINDOW_H
#define ZFLYEMORTHOWINDOW_H

#include <QMainWindow>

class ZDvidTarget;
class ZFlyEmOrthoWidget;
class ZIntPoint;

class ZFlyEmOrthoWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWindow(const ZDvidTarget &target, QWidget *parent = 0);

signals:

public slots:
  void updateData(const ZIntPoint &center);

private:
  ZFlyEmOrthoWidget *m_orthoWidget;
};

#endif // ZFLYEMORTHOWINDOW_H

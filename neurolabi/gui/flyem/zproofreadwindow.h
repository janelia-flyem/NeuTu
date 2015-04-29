#ifndef ZPROOFREADWINDOW_H
#define ZPROOFREADWINDOW_H

#include <QMainWindow>

class ZFlyEmProofMvc;
class QStackedWidget;

class ZProofreadWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZProofreadWindow(QWidget *parent = 0);

  static ZProofreadWindow* Make(QWidget *parent = 0);

signals:
  void splitTriggered(int64_t bodyId);

public slots:
  void launchSplit(int64_t bodyId);
  void exitSplit();

private:
  void init();

private:
  ZFlyEmProofMvc *m_mainMvc;
  QStackedWidget *m_controlGroup;

};

#endif // ZPROOFREADWINDOW_H

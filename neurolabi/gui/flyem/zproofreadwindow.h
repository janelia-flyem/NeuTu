#ifndef ZPROOFREADWINDOW_H
#define ZPROOFREADWINDOW_H

#include <QMainWindow>
#include "tz_stdint.h"

class ZFlyEmProofMvc;
class QStackedWidget;
class ZFlyEmMessageWidget;

class ZProofreadWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZProofreadWindow(QWidget *parent = 0);

  static ZProofreadWindow* Make(QWidget *parent = 0);

signals:
  void splitTriggered(uint64_t bodyId);

public slots:
  void launchSplit(uint64_t bodyId);
  void exitSplit();
  void presentSplitInterface(uint64_t bodyId);

  void dump(const QString &message, bool appending);
  void dumpError(const QString &message, bool appending);

  /*
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void advanceProgress(double dp);
  void endProgress();
  */

private:
  void init();

private:
  ZFlyEmProofMvc *m_mainMvc;
  QStackedWidget *m_controlGroup;
  ZFlyEmMessageWidget *m_messageWidget;

};

#endif // ZPROOFREADWINDOW_H

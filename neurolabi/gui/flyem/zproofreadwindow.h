#ifndef ZPROOFREADWINDOW_H
#define ZPROOFREADWINDOW_H

#include <QMainWindow>

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
  void splitTriggered(int64_t bodyId);

public slots:
  void launchSplit(int64_t bodyId);
  void exitSplit();
  void presentSplitInterface(int64_t bodyId);

  void dump(const QString &message, bool appending);
  void dumpError(const QString &message, bool appending);

private:
  void init();

private:
  ZFlyEmProofMvc *m_mainMvc;
  QStackedWidget *m_controlGroup;
  ZFlyEmMessageWidget *m_messageWidget;

};

#endif // ZPROOFREADWINDOW_H

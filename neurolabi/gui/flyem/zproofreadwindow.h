#ifndef ZPROOFREADWINDOW_H
#define ZPROOFREADWINDOW_H

#include <QMainWindow>
#include <QPalette>

#include "tz_stdint.h"

class ZFlyEmProofMvc;
class QStackedWidget;
class ZFlyEmMessageWidget;
class QProgressDialog;
class ZProgressSignal;
class ZDvidTarget;
class ZWidgetMessage;
class ZDvidDialog;

/*!
 * \brief The mainwindow class of proofreading
 */
class ZProofreadWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZProofreadWindow(QWidget *parent = 0);

  static ZProofreadWindow* Make(QWidget *parent = 0);
  static ZProofreadWindow* Make(QWidget *parent, ZDvidDialog *dvidDlg);

  QProgressDialog* getProgressDialog() {
    return m_progressDlg;
  }

  void setDvidDialog(ZDvidDialog *dvidDlg);

public:
  void test();

signals:
  void splitTriggered(uint64_t bodyId);
//  void splitTriggered();
  /*
  void progressStarted(const QString &title, int nticks);
  void progressStarted(const QString &title);
  void progressAdvanced(double dp);
  void progressEnded();
  */

public slots:
  void launchSplit(uint64_t bodyId);
  void launchSplit();
  void exitSplit();
  void presentSplitInterface(uint64_t bodyId);
  void updateDvidTargetWidget(const ZDvidTarget &target);

  void dump(const QString &message, bool appending = true, bool logging = true);
  void dumpError(const QString &message, bool appending = true,
                 bool logging = true);
  void dump(const ZWidgetMessage &msg);

  void startProgress();
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void advanceProgress(double dp);
  void endProgress();

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void changeEvent(QEvent * event);

private:
  void init();
  void initProgress(int nticks);

  template <typename T>
  void connectMessagePipe(T *source);

  void createMenu();
  void createToolbar();
  void addSynapseActionToToolbar();

  void logMessage(const QString &msg);
  void logMessage(const ZWidgetMessage &msg);

  void displayActiveHint(bool on);

private:
  ZFlyEmProofMvc *m_mainMvc;
  QStackedWidget *m_controlGroup;
  ZFlyEmMessageWidget *m_messageWidget;

  QMenu *m_viewMenu;
  QMenu *m_toolMenu;

  QAction *m_viewSynapseAction;
  QAction *m_viewBookmarkAction;
  QAction *m_viewSegmentationAction;

  QAction *m_importBookmarkAction;
  QAction *m_openSequencerAction;
  QAction *m_contrastAction;

  QAction *m_openSkeletonAction;
  QAction *m_openExtNeuronWindowAction;
  QAction *m_openObject3dAction;

  QToolBar *m_toolBar;
  QToolBar *m_synapseToolbar;

  QProgressDialog *m_progressDlg;
  ZProgressSignal *m_progressSignal;

  QPalette m_defaultPal;
};



#endif // ZPROOFREADWINDOW_H

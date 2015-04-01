#ifndef ZFLYEMROIDIALOG_H
#define ZFLYEMROIDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>
#include <QFuture>

#include "zdialogfactory.h"
#include "flyem/zflyemroiproject.h"
#include "zqtbarprogressreporter.h"
#include "zstackdoc.h"
#include "zintcuboid.h"
#include "zstackdocreader.h"

class MainWindow;
class ZDvidTarget;
class QMenu;

namespace Ui {
class ZFlyEmRoiDialog;
}

/*!
 * \brief The class of ROI control panel
 */
class ZFlyEmRoiDialog : public QDialog, ZProgressable
{
  Q_OBJECT

public:
  explicit ZFlyEmRoiDialog(QWidget *parent = 0);
  ~ZFlyEmRoiDialog();

public:
  void loadGrayscale(int z);
  void loadGrayscale(const ZIntCuboid &box);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  bool appendProject(ZFlyEmRoiProject *project);
  ZFlyEmRoiProject* getProject(size_t index);

  ZFlyEmRoiProject* newProject(const std::string &name);

  void cloneProject(const std::string &name);

  void deleteProject(ZFlyEmRoiProject *project);

  bool isValidName(const std::string &name) const;

public slots:
  void loadGrayscale();
  void setDvidTarget();
  MainWindow* getMainWindow();
  void updateWidget();
  void setDataFrame(ZStackFrame *frame);
  void shallowClearDataFrame();
  void addRoi();

  void loadNextSlice(int currentZ);

  void clear();

  void newDataFrame();
  void updateDataFrame();
  void setZ(int z);
  void previewFullRoi();
  void uploadRoi();
  void estimateRoi();
  void loadProject(int index);
  void dump(const QString &str, bool appending = false);
  void loadSynapse();
  void toggleSynapseView(bool isOn);
  void viewAllSynapseIn3D();

  void runAutoStep(bool ok);
  void setQuickMode(bool quickMode);
  void applyTranslate();

  void processLoadGrayscaleFailure();
  void deleteProject();

signals:
  void newDocReady();
  void progressFailed();
  void progressStart();
  void progressAdvanced(double);
  void progressDone();
  void messageDumped(QString str, bool appending);
  void currentSliceLoaded(int z);

protected:
    void closeEvent(QCloseEvent*event);

private slots:
  void on_searchPushButton_clicked();

  void on_testPushButton_clicked();

  void on_xIncPushButton_clicked();

  void on_xDecPushButton_clicked();

  void on_yDecPushButton_clicked();

  void on_yIncPushButton_clicked();

  void on_rotateLeftPushButton_clicked();

  void on_rotateRightPushButton_clicked();

  void on_xyDecPushButton_clicked();

  void on_xyIncPushButton_clicked();

  void on_movexyDecPushButton_clicked();

  void on_movexyIncPushButton_clicked();

  void on_movexDecPushButton_clicked();

  void on_movexIncPushButton_clicked();

  void on_moveyDecPushButton_clicked();

  void on_moveyIncPushButton_clicked();

  void startProgressSlot();
  void endProgressSlot();
  void advanceProgressSlot(double p);

  void on_pushButton_clicked();

  void on_estimateVolumePushButton_clicked();

  void on_exportPushButton_clicked();

  void on_nextSlicePushButton_clicked();

  void on_prevSlicePushButton_clicked();

  void on_quickPrevPushButton_clicked();

  void on_quickNextPushButton_3_clicked();

  void exportResult();
  void exportRoiObject();
  void exportRoiBlockObject();
  void importRoi();
  void cloneProject();

private:
  void loadGrayscaleFunc(int z, bool lowres);
  void loadPartialGrayscaleFunc(int x0, int x1, int y0, int y1, int z);
  void downloadAllProject();
  void uploadProjectList();
  void createMenu();
  void exportRoiObjectFunc(
      const QString &fileName, int xintv, int yintv, int zintv);
  void exportRoiObjectBlockFunc(const QString &fileName);

  void prepareQuickLoadFunc(
      const ZDvidTarget &target,const std::string &lowresPath, int z);
  void prepareQuickLoad(int z, bool waitForDone = false);
  QString getQuickLoadThreadId(int z) const;
  bool isPreparingQuickLoad(int z) const;


  int getNextZ() const;
  int getPrevZ() const;

  int setNextZ();
  int setPrevZ();

  void quickLoad(int z);

  void startBackgroundJob();
  void closeCurrentProject();

private:
  Ui::ZFlyEmRoiDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZSpinBoxDialog *m_zDlg;
  ZSpinBoxGroupDialog *m_dsDlg;
  QList<ZFlyEmRoiProject*> m_projectList;
  ZFlyEmRoiProject *m_project;
  ZDvidTarget m_dvidTarget;
  ZStackDocReader m_docReader;
  bool m_isLoadingGrayScale;
  bool m_isAutoStepping;

  QMenu *m_nextMenu;
  QMenu *m_mainMenu;

  QAction *m_autoStepAction;
  QAction *m_importRoiAction;
  QAction *m_applyTranslateAction;
  QAction *m_deleteProjectAction;

  int m_xintv;
  int m_yintv;

  QMap<QString, QFuture<void> > m_threadFutureMap;
};

#endif // ZFLYEMROIDIALOG_H

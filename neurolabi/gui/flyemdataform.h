#ifndef FLYEMDATAFORM_H
#define FLYEMDATAFORM_H

#include <QWidget>
#include <QProgressBar>
#include <QImage>
#include <QMap>
#include <QFuture>
#include <QList>

#include "flyem/zflyemneuronlistmodel.h"
#include "zqtbarprogressreporter.h"
#include "flyem/zflyemneuronimagefactory.h"
#include "zwindowfactory.h"

class ZFlyEmNeuronPresenter;
class QStatusBar;
class QMenu;
class ZFlyEmDataFrame;
class ZFlyEmQueryView;
class ZImageWidget;
class QGraphicsScene;
class QMenu;
class SwcExportDialog;
class ZProgressManager;
class QGraphicsItem;

namespace Ui {
class FlyEmDataForm;
}

class FlyEmDataForm : public QWidget
{
  Q_OBJECT
  
public:
  explicit FlyEmDataForm(QWidget *parent = 0);
  ~FlyEmDataForm();
  
  virtual QSize sizeHint() const;

  void appendOutput(const QString &text);
  void setQueryOutput(const ZFlyEmNeuron *neuron);
  void appendQueryOutput(const ZFlyEmNeuron *neuron);

  QProgressBar* getProgressBar();
  ZProgressManager* getProgressManager() {
    return m_progressManager;
  }

  void setPresenter(ZFlyEmNeuronPresenter *presenter);

  inline void setStatusBar(QStatusBar *bar) { m_statusBar = bar; }

  ZFlyEmDataFrame* getParentFrame() const;

  /*!
   * \brief Create all context menus
   */
  void createContextMenu();

  /*!
   * \brief Create all actions
   */
  void createAction();

  /*!
   * \brief Update the view of query table
   */
  void updateQueryTable();

  void updateSlaveQueryTable();

  void dump(const QString &message);

  QList<int> getSelectedNeuronList() const;

protected:
  void resizeEvent(QResizeEvent *);
  void showEvent(QShowEvent *);

  void initThumbnailScene();
  void createMenu();
  void createExportMenu();
  void createImportMenu();

signals:
  void showSummaryTriggered();
  void processTriggered();
  void queryTriggered();
  void testTriggered();
  void generalTriggered();
  void optionTriggered();
  void volumeTriggered(const QString&);
  void saveBundleTriggered(int, const QString&);
  void showNearbyNeuronTriggered(const ZFlyEmNeuron *neuron);
  void searchNeighborNeuronTriggered(const ZFlyEmNeuron *neuron);
  void thumbnailItemReady(QList<QGraphicsItem*> itemList, int bodyId);

private slots:
  void on_pushButton_clicked();

  void on_processPushButton_clicked();

  void on_queryPushButton_clicked();

  void on_testPushButton_clicked();

  void on_generalPushButton_clicked();

  void on_optionPushButton_clicked();

  void on_addDataPushButton_clicked();

  void on_savePushButton_clicked();

  void updateStatusBar(const QModelIndex &index);
  void updateInfoWindow(const QModelIndex &index);
  void viewModel(const QModelIndex &index);
  void showSelectedModel();
  void updateSelectedModel();
  void showSelectedBody();
  //void showSelectedBodyCoarse();
  void showSelectedModelWithBoundBox();
  void showNearbyNeuron();
  void searchNeighborNeuron();
  void updateSlaveQuery(const QModelIndex &index);
  void showSecondarySelectedModel();
  void updateThumbnail(const QModelIndex &index);
  void updateThumbnailSecondary(const QModelIndex &index);

  void updateThumbnail(QList<QGraphicsItem *> itemList, int bodyId);
  void generateThumbnailItem(
      QList<QGraphicsItem *> currentItemList, ZFlyEmNeuron *neuron);

  /*!
   * \brief Change class of selected neurons
   *
   * Only the selected neuron keys are taken as input
   */
  void changeNeuronClass();

  /*!
   * \brief Assign class to a neuron
   *
   * The function only work on the primary neuron.
   *
   * \param index Model index.
   */
  void assignClass(const QModelIndex &index);

  void on_showModelPushButton_clicked();

  void on_saveSwcPushButton_clicked();

  void on_exportPushButton_clicked();

  void exportVolumeRenderingFigure();

  void exportTypeLabelFile();

  void importSynapse();

  void slotTest();

private:
  ZStackDoc* showViewSelectedModel(ZFlyEmQueryView *view);
  ZStackDoc* updateViewSelectedModel(ZFlyEmQueryView *view);
  ZStackDoc* showViewSelectedBody(ZFlyEmQueryView *view);
  void updateThumbnail(ZFlyEmNeuron *neuron, bool computing = true);
  void updateThumbnailLive(ZFlyEmNeuron *neuron);
  uint64_t computeThumbnailFunc(ZFlyEmNeuron *neuron);
  void saveVolumeRenderingFigure(
      ZFlyEmNeuron *neuron, const QString &output, const QString cameraFile);
  Stack *loadThumbnailImage(ZFlyEmNeuron *neuron);

private:
  Ui::FlyEmDataForm *ui;
  ZFlyEmNeuronListModel *m_neuronList;
  ZFlyEmNeuronListModel *m_secondaryNeuronList;
  QStatusBar *m_statusBar;

  QMenu *m_neuronContextMenu;
  QAction *m_showSelectedModelAction;
  QAction *m_updateSelectedModelAction;
  QAction *m_showSelectedModelWithBoundBoxAction;
  QAction *m_changeClassAction;
  QAction *m_neighborSearchAction;
  QAction *m_showSelectedBodyAction;

  QMenu *m_secondaryNeuronContextMenu;
  QAction *m_showSecondarySelectedModelAction;

  ZQtBarProgressReporter m_specialProgressReporter;

  /*
  QImage *m_thumbnailImage;
  ZImageWidget *m_thumbnailWidget;
  */
  QGraphicsScene *m_thumbnailScene;
  //ZFlyEmNeuronImageFactory m_imageFactory;

//  QMap<QString, QFuture<void> > m_threadFutureMap;

  QMap<QString, QFuture<uint64_t> >m_bodyFutureMap;
  QFutureWatcher<uint64_t> m_thumbnailFutureWatcher;
  QMenu *m_mainMenu;
  QMenu *m_exportMenu;
  QMenu *m_importMenu;

  SwcExportDialog *m_swcExportDlg;

  ZProgressManager *m_progressManager;
  ZWindowFactory m_3dWindowFactory;
};

#endif // FLYEMDATAFORM_H

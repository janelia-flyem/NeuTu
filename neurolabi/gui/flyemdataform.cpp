#include "flyemdataform.h"
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QTextBrowser>
#include <QListView>
#include <QInputDialog>
#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <QGraphicsPixmapItem>

#include "neutubeconfig.h"
#include "tz_error.h"
#include "zswctree.h"
#include "zstackdoc.h"
#include "ui_flyemdataform.h"
#include "flyem/zflyemdataframe.h"
#include "flyem/zflyemstackframe.h"
#include "zimagewidget.h"
#include "zfiletype.h"
#include "zimage.h"

FlyEmDataForm::FlyEmDataForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmDataForm),
  m_statusBar(NULL),
  m_neuronContextMenu(NULL),
  m_showSelectedModelAction(NULL),
  m_changeClassAction(NULL),
  m_neighborSearchAction(NULL),
  m_secondaryNeuronContextMenu(NULL),
  m_showSecondarySelectedModelAction(NULL)
{
  ui->setupUi(this);
  ui->slaveQueryView->setParent(ui->InformationTab->widget(1));

  ui->progressBar->hide();
  m_neuronList = new ZFlyEmNeuronListModel(this);
  ui->queryView->setModel(m_neuronList);
  m_secondaryNeuronList = new ZFlyEmNeuronListModel(this);
  ui->slaveQueryView->setModel(m_secondaryNeuronList);
  ui->queryView->setSelectionMode(QAbstractItemView::ExtendedSelection);
#ifdef _DEBUG_
  connect(ui->queryView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(assignClass(QModelIndex)));
#endif
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateStatusBar(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateInfoWindow(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateSlaveQuery(QModelIndex)));
  connect(ui->queryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateThumbnail(QModelIndex)));
  connect(ui->slaveQueryView, SIGNAL(clicked(QModelIndex)),
          this, SLOT(updateThumbnailSecondary(QModelIndex)));

  //customize
  //ui->testPushButton->hide();
  ui->generalPushButton->hide();

  createAction();
  createContextMenu();
  ui->queryView->setContextMenu(m_neuronContextMenu);
  ui->slaveQueryView->setContextMenu(m_secondaryNeuronContextMenu);

  m_specialProgressReporter.setProgressBar(ui->progressBar);
  setProgressReporter(&m_specialProgressReporter);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  //m_thumbnailImage = new QImage;
  //m_thumbnailImage->load(":/images/hideobj.png");
  m_thumbnailScene = new QGraphicsScene;
  ui->thumbnailView->setScene(m_thumbnailScene);
  /*
  m_thumbnailWidget = new ZImageWidget(ui->thumnailContainer, m_thumbnailImage);
  m_thumbnailWidget->setProjRegion(
        QRect(QPoint(0, 0), ui->thumnailContainer->size()));
        */

  ui->overallLayout->setMargin(10);
  ui->thumbnailView->resize(256, ui->thumbnailView->height());
  ui->outputTextEdit->resize(190, ui->outputTextEdit->height());
  setLayout(ui->overallLayout);

}

FlyEmDataForm::~FlyEmDataForm()
{
  //delete m_thumbnailImage;
  delete ui;
  delete m_thumbnailScene;
}

QSize FlyEmDataForm::sizeHint() const
{
#ifdef _DEBUG_2
  std::cout << geometry().x() << " " << geometry().y()
            << geometry().width() << " " << geometry().height() << std::endl;
#endif

  return geometry().size();
}

void FlyEmDataForm::on_pushButton_clicked()
{
  emit showSummaryTriggered();
}

void FlyEmDataForm::on_processPushButton_clicked()
{
  emit processTriggered();
}

void FlyEmDataForm::on_queryPushButton_clicked()
{
  emit queryTriggered();
}

void FlyEmDataForm::appendOutput(const QString &text)
{
  ui->outputTextEdit->append(text);
}

void FlyEmDataForm::setQueryOutput(const ZFlyEmNeuron *neuron)
{
  m_neuronList->clear();
  appendQueryOutput(neuron);
}

void FlyEmDataForm::appendQueryOutput(const ZFlyEmNeuron *neuron)
{
  //ui->queryTextWidget->append(neuron.c_str());
  if (neuron != NULL) {
    m_neuronList->append(neuron);
  }
}

void FlyEmDataForm::on_testPushButton_clicked()
{
  emit testTriggered();
}

QProgressBar* FlyEmDataForm::getProgressBar()
{
  return ui->progressBar;
}

void FlyEmDataForm::on_generalPushButton_clicked()
{
  emit generalTriggered();
}

void FlyEmDataForm::on_optionPushButton_clicked()
{
  emit optionTriggered();
}

void FlyEmDataForm::on_addDataPushButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(
        this, tr("Load FlyEM Database"), "", tr("Json files (*.json)"),
        NULL/*, QFileDialog::DontUseNativeDialog*/);

  if (!fileName.isEmpty()) {
    ZFlyEmDataFrame *frame = dynamic_cast<ZFlyEmDataFrame*>(this->parentWidget());
    if (frame != NULL) {
      frame->load(fileName.toStdString(), true);
    }
  }
}

void FlyEmDataForm::on_savePushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Query Output"), "", tr("Text files (*.csv)"),
        NULL);
  if (!fileName.isEmpty()) {
    m_neuronList->exportCsv(fileName);
  }
}

void FlyEmDataForm::updateStatusBar(const QModelIndex &index)
{
  if (m_statusBar != NULL) {
    m_statusBar->showMessage(
          m_neuronList->data(index, Qt::StatusTipRole).toString());
  }
}

void FlyEmDataForm::updateInfoWindow(const QModelIndex &index)
{
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

  ui->infoWindow->clear();
  foreach (const ZFlyEmNeuron* neuron, neuronArray) {
    std::ostringstream stream;
    neuron->print(stream);
    ui->infoWindow->append(stream.str().c_str());
  }
}

ZFlyEmDataFrame* FlyEmDataForm::getParentFrame() const
{
  return dynamic_cast<ZFlyEmDataFrame*>(this->parentWidget());
}

void FlyEmDataForm::updateSlaveQuery(const QModelIndex &index)
{
  ZFlyEmDataFrame *frame = getParentFrame();
  if (frame != NULL) {
    if (m_neuronList->getColumnName(index.column()) == "Class") {
      const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index.row());
      if (neuron->hasClass()) {
        m_secondaryNeuronList->clear();
        ZFlyEmDataBundle *bundle = frame->getDataBundle();
        std::vector<ZFlyEmNeuron> &neuronArray = bundle->getNeuronArray();
        for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
             iter != neuronArray.end(); ++iter) {
          ZFlyEmNeuron &buddyNeuron = *iter;
          if (buddyNeuron.getClass() == neuron->getClass()) {
            m_secondaryNeuronList->append(&buddyNeuron);
          }
        }
      }
    }
  }
}

void FlyEmDataForm::assignClass(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in assignClass");
    return;
  }

  if (index.column() == 0) {
    ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
    if (neuron != NULL) {
      const std::vector<const ZFlyEmNeuron*> &topMatch = neuron->getTopMatch();
      QString className = "";
      if (!topMatch.empty()) {
        className = topMatch[0]->getClass().c_str();
      }

      //Popup dialog
      bool ok;
      className = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                        tr("Class name:"), QLineEdit::Normal,
                                        className, &ok);
      if (ok) {
        neuron->setClass(className.toStdString());
        m_neuronList->notifyRowDataChanged(index.row());
      }
    }
  }
}

void FlyEmDataForm::viewModel(const QModelIndex &index)
{
  if (index.row() > m_neuronList->rowCount()) {
    appendOutput("Invalid index in viewSwc");
    return;
  }

  ui->progressBar->setValue(50);
  ui->progressBar->show();
  //QApplication::processEvents();

  //const ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  QVector<const ZFlyEmNeuron*> neuronArray =
      m_neuronList->getNeuronArray(index);

#ifdef _DEBUG_2
  std::cout << m_neuronList->headerData(index.column(), Qt::Horizontal).toString().toStdString()
             << std::endl;

#endif

  if (!neuronArray.isEmpty()) {
    ZStackFrame *frame = new ZStackFrame;

    foreach (const ZFlyEmNeuron* neuron, neuronArray) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZSwcTree *model = neuron->getModel();
      ZStackDoc *doc = frame->document().get();
      doc->blockSignals(true);
      if (model != NULL) {
        doc->addSwcTree(model->clone(), true);
      }

      std::vector<ZPunctum*> puncta = neuron->getSynapse();
      for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        doc->addPunctum(*iter);
      }
      doc->blockSignals(false);
      doc->updateModelData(ZStackDoc::SWC_DATA);
      doc->updateModelData(ZStackDoc::PUNCTA_DATA);
    }

    frame->open3DWindow(this->parentWidget());
    delete frame;
  } else if (m_neuronList->headerData(index.column(), Qt::Horizontal) ==
             "Volume Path") {
    if (!m_neuronList->data(index).toString().isEmpty()) {
      emit volumeTriggered(m_neuronList->data(index).toString());
    }
  }

  ui->progressBar->hide();
}

void FlyEmDataForm::showViewSelectedModel(ZFlyEmQueryView *view)
{
  ui->progressBar->setValue(50);
  ui->progressBar->show();
  //QApplication::processEvents();

  QItemSelectionModel *sel = view->selectionModel();

#ifdef _DEBUG_2
  appendOutput(QString("%1 rows selected").arg(sel->selectedIndexes().size()).toStdString());
#endif

  ZStackFrame *frame = new ZStackFrame;

  view->getModel()->retrieveModel(sel->selectedIndexes(), frame->document().get());
  ui->progressBar->setValue(75);
  //QApplication::processEvents();

  frame->open3DWindow(this->parentWidget());
  delete frame;

  ui->progressBar->hide();
}

void FlyEmDataForm::showSelectedModel()
{
  showViewSelectedModel(ui->queryView);
}

void FlyEmDataForm::showSecondarySelectedModel()
{
  showViewSelectedModel(ui->slaveQueryView);
}

void FlyEmDataForm::on_showModelPushButton_clicked()
{
  showSelectedModel();
}

void FlyEmDataForm::setPresenter(ZFlyEmNeuronPresenter *presenter)
{
  m_neuronList->setPresenter(presenter);
}

void FlyEmDataForm::on_saveSwcPushButton_clicked()
{
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Export SWC"),
    "", QFileDialog::ShowDirsOnly);

  if (!dirpath.isEmpty()) {
    m_neuronList->exportSwc(dirpath);
  }
}

void FlyEmDataForm::on_exportPushButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save bundle"), "", tr("JSON files (*.json)"),
        NULL);
  if (!fileName.isEmpty()) {
    emit saveBundleTriggered(0, fileName);
  }
}

void FlyEmDataForm::changeNeuronClass()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();
  int neuronCount = 0;

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      ++neuronCount;
    }
  }

  if (neuronCount > 0) {
    bool ok;

    QString className = QInputDialog::getText(
          this, QString("%1 neurons: Change Class").arg(neuronCount),
          tr("Class name:"), QLineEdit::Normal,
          "", &ok);
    if (ok) {
      foreach (QModelIndex index, indexList) {
        if (m_neuronList->isNeuronKey(index)) {
          ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
          if (neuron != NULL) {
            neuron->setClass(className.toStdString());
            m_neuronList->notifyRowDataChanged(index.row());
          }
        }
      }
    }
  } else {
    QMessageBox::warning(this,
          "No Neuron Selected",
          "At least one neuron key should be selected to change class");
  }
}

void FlyEmDataForm::createContextMenu()
{
  if (m_neuronContextMenu == NULL) {
    m_neuronContextMenu = new QMenu(this);
    m_neuronContextMenu->addAction(m_showSelectedModelAction);
    m_neuronContextMenu->addAction(m_changeClassAction);
    m_neuronContextMenu->addAction(m_neighborSearchAction);
  }

  if (m_secondaryNeuronContextMenu == NULL) {
    m_secondaryNeuronContextMenu = new QMenu(this);
    m_secondaryNeuronContextMenu->addAction(m_showSecondarySelectedModelAction);
  }
}

void FlyEmDataForm::createAction()
{
  if (m_showSelectedModelAction == NULL) {
    m_showSelectedModelAction = new QAction("Show Model", this);
    connect(m_showSelectedModelAction, SIGNAL(triggered()),
            this, SLOT(showSelectedModel()));
  }

  if (m_showSecondarySelectedModelAction == NULL) {
    m_showSecondarySelectedModelAction = new QAction("Show Model", this);
    connect(m_showSecondarySelectedModelAction, SIGNAL(triggered()),
            this, SLOT(showSecondarySelectedModel()));
  }

  if (m_changeClassAction == NULL) {
    m_changeClassAction = new QAction("Change Class", this);
    connect(m_changeClassAction, SIGNAL(triggered()),
            this, SLOT(changeNeuronClass()));
  }

  if (m_neighborSearchAction == NULL) {
    m_neighborSearchAction = new QAction("Search neighbor", this);
    connect(m_neighborSearchAction, SIGNAL(triggered()),
            this, SLOT(searchNeighborNeuron()));
  }
}

void FlyEmDataForm::showNearbyNeuron()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      emit showNearbyNeuronTriggered(m_neuronList->getNeuron(index));
      break;
    }
  }
}

void FlyEmDataForm::searchNeighborNeuron()
{
  QItemSelectionModel *sel = ui->queryView->selectionModel();

  QModelIndexList indexList = sel->selectedIndexes();

  foreach (QModelIndex index, indexList) {
    if (m_neuronList->isNeuronKey(index)) {
      emit searchNeighborNeuronTriggered(m_neuronList->getNeuron(index));
      break;
    }
  }
}

void FlyEmDataForm::updateQueryTable()
{
  m_neuronList->notifyAllDataChanged();
}

void FlyEmDataForm::updateSlaveQueryTable()
{
  m_secondaryNeuronList->notifyAllDataChanged();
}

void FlyEmDataForm::updateThumbnail(const QModelIndex &index)
{
  ZFlyEmNeuron *neuron = m_neuronList->getNeuron(index);
  updateThumbnail(neuron);
}

void FlyEmDataForm::updateThumbnailSecondary(const QModelIndex &index)
{ 
  ZFlyEmNeuron *neuron = m_secondaryNeuronList->getNeuron(index);
  updateThumbnail(neuron);
}

void FlyEmDataForm::updateThumbnail(ZFlyEmNeuron *neuron)
{
  m_thumbnailScene->clear();
  if (neuron != NULL) {
    if (!neuron->getThumbnailPath().empty()) {
      QGraphicsPixmapItem *thumbnailItem = new QGraphicsPixmapItem;
      QPixmap pixmap;
      if (!pixmap.load(neuron->getThumbnailPath().c_str())) {
        if (ZFileType::fileType(neuron->getThumbnailPath()) ==
            ZFileType::TIFF_FILE) {
          Stack *stack = C_Stack::readSc(neuron->getThumbnailPath().c_str());
          if (stack != NULL) {
            ZImage image(C_Stack::width(stack), C_Stack::height(stack));
            image.setData(C_Stack::array8(stack));
#ifdef _DEBUG_2
            image.save((GET_DATA_DIR + "/test.png").c_str());
#endif
            if (!pixmap.convertFromImage(image)) {
              dump("Failed to load the thumbnail.");
            }
            C_Stack::kill(stack);
          } else {
            dump("Failed to load the thumbnail.");
          }
        }
      }
      thumbnailItem->setPixmap(pixmap);
      m_thumbnailScene->addItem(thumbnailItem);
    }
  }
}

void FlyEmDataForm::dump(const QString &message)
{
  appendOutput("<p>" + message + "</p>");
  //QApplication::processEvents();
}

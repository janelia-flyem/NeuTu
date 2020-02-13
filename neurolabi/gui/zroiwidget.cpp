
#include <vector>
#include <string>
#include <iostream>

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QGroupBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "neutubeconfig.h"
#include "flyem/zflyemmisc.h"
#include "zstackobjectsourcefactory.h"
#include "z3dwindow.h"
#include "z3dsurfacefilter.h"
#include "zroiwidget.h"
#include "flyem/roi/zroimesh.h"
#include "qfonticon.h"
#include "flyem/roi/zroiitemmodel.h"
//#include "flyem/zflyemproofdoc.h"
//

#if 0
ZROIObjsModel::ZROIObjsModel(QObject *parent) : ZObjsModel(parent)
{
}

ZROIObjsModel::~ZROIObjsModel()
{
}

void ZROIObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
    ZObjsModel::setModelIndexCheckState(index, cs);
}

bool ZROIObjsModel::needCheckbox(const QModelIndex &index) const
{
    if (index.isValid()) {
        return true;
    }

    QModelIndex idx = parent(index);
    if (idx.isValid() && static_cast<ZObjsItem*>(idx.internalPointer()) == m_rootItem) {
        return true;
    }
    else{
        return false;
    }
}
#endif

//
ZROIWidget::ZROIWidget(QWidget *parent) : QDockWidget(parent)
{
//    m_roiList.clear();
//    m_defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
//    m_roiList.clear();
//    m_defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    //
//    m_objmodel = new ZROIObjsModel(this);
}

ZROIWidget::~ZROIWidget()
{

}

void ZROIWidget::closeEvent(QCloseEvent * /*event*/)
{
    emit toBeClosed();
}

void ZROIWidget::initRoiView(
    Z3DWindow *window,
    const std::shared_ptr<ZRoiProvider> &roiProvider)
{
  if (!m_roiProvider) {
    m_roiProvider = roiProvider;
    m_roiItemModel = new ZRoiItemModel(this);
    m_roiItemModel->setRoiProvider(roiProvider);
    connect(m_roiItemModel, &ZRoiItemModel::roiUpdated,
            this, &ZROIWidget::roiUpdated);
    connect(m_roiItemModel, &ZRoiItemModel::roiListUpdated,
            this, &ZROIWidget::roiListUpdated);

    m_roiItemModelProxy = new QSortFilterProxyModel(this);
    m_roiItemModelProxy->setSourceModel(m_roiItemModel);
    m_roiItemModelProxy->setFilterKeyColumn(ZRoiItemModel::COLUMN_NAME);

    m_roiTable = new QTableView(this);
    m_roiTable->setModel(m_roiItemModelProxy);
    m_roiTable->setSortingEnabled(true);
    m_roiTable->sortByColumn(ZRoiItemModel::COLUMN_NAME, Qt::AscendingOrder);

    m_window = window;
    if (m_window) {
      m_window->registerRoiWidget(this);
    }

    makeGui();
  }
}

#if 0
void ZROIWidget::loadROIs(Z3DWindow *window,
//                         const ZDvidInfo &dvidInfo,
                         const std::vector<std::shared_ptr<ZRoiMesh> > &roiList)
{
    //
    m_window = window;
//    m_dvidInfo =  dvidInfo;
    loadROIs(roiList);
}

void ZROIWidget::loadROIs(const std::vector<std::shared_ptr<ZRoiMesh> > &roiList)
{
  m_roiList = roiList;
  /*
  m_roiList = roiList;
  m_loadedROIs = loadedROIs;
  m_roiSourceList = roiSourceList;
  */

  /*
  m_roiList.resize(roiList.size());
  for (size_t i = 0; i < roiList.size(); ++i) {
    ZRoiMesh *roiMesh = new ZRoiMesh;
    roiMesh->setName(roiList[i]);
    if (loadedROIs.size() > i) {
      roiMesh->setMesh(loadedROIs[i]);
    }
    m_roiList[i] = std::shared_ptr<ZRoiMesh>(roiMesh);
  }
  */

  if (m_window != NULL) {
    m_colorModified.resize(m_roiList.size(), false);
    makeGUI();
  }
}


void ZROIWidget::setCheckStatus(int row, bool on)
{
  if (row >= 0 && row < (int) m_checkStatus.size()) {
    m_checkStatus[row] = on;

    QTableWidgetItem *item = m_roiTableView->item(row, 0);
    item->setCheckState(on ? Qt::Checked : Qt::Unchecked);
  }
}

void ZROIWidget::toggleCheckStatus(int row)
{
  if (row >= 0 && row < (int) m_checkStatus.size()) {
    bool on = !m_checkStatus[row];
    setCheckStatus(row, on);
  }
}
#endif

int ZROIWidget::getSourceModelIndex(int row) const
{
  return m_roiItemModelProxy->mapToSource(
        m_roiItemModelProxy->index(row, 0)).row();
}

std::vector<int> ZROIWidget::getCurrentSourceModelIndexList() const
{
  int itemCount = m_roiItemModelProxy->rowCount();
  std::vector<int> indexList;
  for (int i = 0; i < itemCount; ++i) {
    indexList.push_back(getSourceModelIndex(i));
  }

  return indexList;
}

/*
void ZROIWidget::updateRoiTable()
{
  m_roiTableView->clear();
  size_t roiCount = m_roiList.size();
  for (std::size_t i = 0; i < roiCount; ++i)
  {
    QTableWidgetItem *roiNameItem = new QTableWidgetItem(
          m_roiList[i]->getName().c_str());
    roiNameItem->setFlags(roiNameItem->flags() ^ Qt::ItemIsEditable);
    roiNameItem->setCheckState(Qt::Unchecked);

    ZColorScheme zcolor;
    zcolor.setColorScheme(ZColorScheme::RANDOM_COLOR);
    QBrush brush(zcolor.getColor((uint64_t) i));

    QTableWidgetItem *colorItem = new QTableWidgetItem(tr("@COLOR"));
    colorItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    colorItem->setFlags(colorItem->flags() ^ Qt::ItemIsEditable);
    QFont font;
    font.setBold(true);
    colorItem->setFont(font);
    //colorItem->setBackgroundColor(defaultColor);
    colorItem->setForeground(brush);

    int row = m_roiTableView->rowCount();
    m_roiTableView->insertRow(row);
    m_roiTableView->setItem(row, 0, roiNameItem);
    m_roiTableView->setItem(row, 1, colorItem);

//    bool checked = false;
//    m_checkStatus.push_back(checked);
  }
  m_checkStatus.resize(roiCount, false);
}
*/

void ZROIWidget::makeGui()
{
  m_toggleButton = new QPushButton("");
  m_toggleButton->setToolTip("Toggle all in the current list");
  m_toggleButton->setIcon(QFontIcon::icon(0xf192, Qt::darkGreen));

  m_showAllButton = new QPushButton("");
  m_showAllButton->setToolTip("Show all in the current list");
  m_showAllButton->setIcon(QFontIcon::icon(0xf06e, Qt::darkGreen));

  m_hideAllButton = new QPushButton();
  m_hideAllButton->setToolTip("Hide all in the current list");
  m_hideAllButton->setIcon(QFontIcon::icon(0xf070, Qt::darkRed));


  double alpha = m_window->getFilter(neutu3d::ERendererLayer::ROI)->opacity();
//   double alpha = m_window->getSurfaceFilter()->getOpacity();
  m_opacityLabel = new QLabel;//(tr(" Opacity: %1").arg(alpha, 0, 'f', 2, '0'));
  updateOpacityLabel(alpha);
  m_opacitySlider = new QSlider(Qt::Horizontal);
  m_opacitySlider->setRange(0,100);
  m_opacitySlider->setValue(alpha*100);

  QHBoxLayout *hLayout = new QHBoxLayout;

  hLayout->addWidget(m_opacityLabel);
  hLayout->addWidget(m_opacitySlider);

//  QGroupBox *horizontalGroupBox = new QGroupBox();
//  horizontalGroupBox->setLayout(hLayout);

  QVBoxLayout *layout = new QVBoxLayout();

  layout->addLayout(hLayout);

//  QHBoxLayout *controlLayout = new QHBoxLayout();


//  controlLayout->addWidget(horizontalGroupBox);

//  layout->addLayout(controlLayout);

  m_filterEdit = new QLineEdit(this);
  m_filterEdit->setPlaceholderText("Filter ROIs...");
  connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(filterRoi()));

  m_filterRegexCheckBox = new QCheckBox(this);
  m_filterRegexCheckBox->setChecked(false);
  m_filterRegexCheckBox->setText("RegEx");
  connect(m_filterRegexCheckBox, SIGNAL(stateChanged(int)),
          this, SLOT(filterRoi()));


  QHBoxLayout *hlayout2 = new QHBoxLayout();
  hlayout2->addWidget(m_toggleButton);
  hlayout2->addWidget(m_showAllButton);
  hlayout2->addWidget(m_hideAllButton);

  QHBoxLayout *hlayout3 = new QHBoxLayout();
  hlayout3->addWidget(m_filterEdit);
  hlayout3->addWidget(m_filterRegexCheckBox);

  layout->addLayout(hlayout2);
  layout->addLayout(hlayout3);

  layout->addWidget(m_roiTable);

  QGroupBox *group = new QGroupBox();
  group->setLayout(layout);

#ifdef _DEBUG_
  std::cout << "Set ROI widget" << std::endl;
#endif

  this->setWidget(group);

  connect(m_roiTable, SIGNAL(doubleClicked(const QModelIndex&)),
          this, SLOT(handleRoiTableDoubleClick(const QModelIndex&)));
  connect(m_toggleButton, SIGNAL(clicked()), this, SLOT(toggleSelection()));
  connect(m_showAllButton, SIGNAL(clicked()), this, SLOT(showAllRoi()));
  connect(m_hideAllButton, SIGNAL(clicked()), this, SLOT(hideAllRoi()));
  connect(m_opacitySlider,SIGNAL(valueChanged(int)),this,SLOT(updateSlider(int)));
  connect(m_window->getFilter(neutu3d::ERendererLayer::ROI), SIGNAL(opacityChanged(double)),
          this,SLOT(updateOpacity(double)));
}

#if 0
void ZROIWidget::makeGUI()
{
    if(m_roiList.empty())
        return;

    //
    m_selectAll = new QCheckBox("Select All");
    m_selectAll->setChecked(false);

    m_toggleButton = new QPushButton("Toggle");
    m_toggleButton->setIcon(QFontIcon::icon(0xf06e, Qt::darkGreen));

    //
    double alpha = m_window->getFilter(neutu3d::ERendererLayer::ROI)->opacity();
 //   double alpha = m_window->getSurfaceFilter()->getOpacity();
    m_opacityLabel = new QLabel;//(tr(" Opacity: %1").arg(alpha, 0, 'f', 2, '0'));
    updateOpacityLabel(alpha);
    m_opacitySlider = new QSlider(Qt::Horizontal);
    m_opacitySlider->setRange(0,100);
    m_opacitySlider->setValue(alpha*100);

    //
    QHBoxLayout *hLayout = new QHBoxLayout;
//    hLayout->addWidget(m_selectAll);
//    hLayout->addStretch();
    hLayout->addWidget(m_opacityLabel);
    hLayout->addWidget(m_opacitySlider);

    QGroupBox *horizontalGroupBox = new QGroupBox();
    horizontalGroupBox->setLayout(hLayout);

    //
    m_roiTableView = new QTableWidget(0, 2);
    m_roiTableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("ROI Name") << tr("Color");
    m_roiTableView->setHorizontalHeaderLabels(labels);

#ifdef _QT5_
    m_roiTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    m_roiTableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif
//    tw_ROIs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    m_roiTableView->verticalHeader()->hide();
    m_roiTableView->setShowGrid(false);

    //
    //QBrush brush(defaultColor);
    updateRoiTable();

    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *controlLayout = new QHBoxLayout();


    controlLayout->addWidget(horizontalGroupBox);

    layout->addLayout(controlLayout);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Filter ROIs...");
    connect(m_filterEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(filterRoi(const QString &)));

    QHBoxLayout *hlayout2 = new QHBoxLayout();

    hlayout2->addWidget(m_toggleButton);
    hlayout2->addWidget(m_filterEdit);

    layout->addLayout(hlayout2);

    layout->addWidget(m_roiTableView);

    QGroupBox *group = new QGroupBox();
    group->setLayout(layout);

#ifdef _DEBUG_
    std::cout << "Set ROI widget" << std::endl;
#endif

    this->setWidget(group);

    //
    //connect(tw_ROIs, SIGNAL(cellClicked(int,int)), this, SLOT(updateROISelections(int,int)));
    connect(m_roiTableView, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(updateROIColors(int,int)));
    connect(m_roiTableView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(updateROISelections(QModelIndex)));
    connect(m_selectAll, SIGNAL(clicked()), this, SLOT(updateSelection()));
    connect(m_toggleButton, SIGNAL(clicked()), this, SLOT(toggleSelection()));
    connect(m_opacitySlider,SIGNAL(valueChanged(int)),this,SLOT(updateSlider(int)));
    connect(m_window->getFilter(neutu3d::ERendererLayer::ROI), SIGNAL(opacityChanged(double)),
            this,SLOT(updateOpacity(double)));
}
#endif

#if 0
void ZROIWidget::updateROISelections(QModelIndex idx)
{
    int row = idx.row();
    int col = idx.column();
    if (col == 0) {
      toggleCheckStatus(row);
      updateROIs();
    }
#if 0
    //
    if(col==0)
    {
        //
        QTableWidgetItem *item = tw_ROIs->item(row, 0);

        //
        if(m_checkStatus[row] == true)
        {
            m_checkStatus[row] = false;
            item->setCheckState(Qt::Unchecked);
        }
        else
        {
            m_checkStatus[row] = true;
            item->setCheckState(Qt::Checked);
        }

        //
        updateROIs();

    }
    else
    {
        // edit color
    }
#endif
}
#endif

int ZROIWidget::getDsIntv() const
{
  return 0;
//  return m_dsIntvWidget->value();
}

void ZROIWidget::showAllRoi()
{
  m_roiItemModel->setChecked(getCurrentSourceModelIndexList(), true);
}

void ZROIWidget::hideAllRoi()
{
  m_roiItemModel->setChecked(getCurrentSourceModelIndexList(), false);
}

void ZROIWidget::toggleSelection()
{
//  QString filterString = m_filterEdit->text();

  int itemCount = m_roiItemModelProxy->rowCount();
//  int checkedCount = 0;
  std::vector<int> indexList;
  for (int i = 0; i < itemCount; ++i) {
    int index = getSourceModelIndex(i);
    indexList.push_back(index);
//    m_roiItemModel->setChecked(index, !m_roiItemModel->isChecked(index));
    /*
    QVariant data = m_roiItemModelProxy->data(
          m_roiItemModelProxy->index(i, ZRoiItemModel::COLUMN_NAME),
          Qt::CheckStateRole);
    if (static_cast<Qt::CheckState>(data.toInt()) == Qt::Checked) {
      ++checkedCount;
    }
    */
  }

  m_roiItemModel->toggleChecked(indexList);

  /*
  Qt::CheckState newCheckState = Qt::Unchecked;
  if (checkedCount * 2 < itemCount) {
    newCheckState = Qt::Checked;
  }

  for (int i = 0; i < itemCount; ++i) {
    m_roiItemModelProxy->setData(
          m_roiItemModelProxy->index(i, ZRoiItemModel::COLUMN_NAME),
          newCheckState, Qt::CheckStateRole);
  }
  */
//  m_newRoiTableView->repaint();

  /*
  std::vector<QAbstractS> itemList(itemCount, nullptr);
  for(int i = 0; i < count; i++) {
//    if (filterString.isEmpty() || getRoiName(i).contains(filterString)) {
      itemList.push_back(m_newRoiTableView->item(i, 0));
//    }
  }

  size_t checkedCount = 0;
  for (QTableWidgetItem *item : itemList) {
    if (item->checkState() == Qt::Checked) {
      ++checkedCount;
    }
  }

  Qt::CheckState newCheckState = Qt::Unchecked;
  if (checkedCount * 2 < itemList.size()) {
    newCheckState = Qt::Checked;
  }

  for (QTableWidgetItem *item : itemList) {
    item->setCheckState(newCheckState);
  }
  */

//  updateROIs();
}

#if 0

void ZROIWidget::updateSelection()
{
    if(m_selectAll->isChecked())
    {
        for(int i=0; i<m_roiTableView->rowCount(); i++)
        {
            QTableWidgetItem *item = m_roiTableView->item(i, 0);
            item->setCheckState(Qt::Checked);
        }
    }
    else
    {
        for(int i=0; i<m_roiTableView->rowCount(); i++)
        {
            QTableWidgetItem *item = m_roiTableView->item(i, 0);
            item->setCheckState(Qt::Unchecked);
        }
    }

    updateROIs();
}


QTableWidgetItem* ZROIWidget::getRoiItem(int index) const
{
  return m_roiTableView->item(index, 0);
}

QColor ZROIWidget::getRoiColor(int index) const
{
  return m_roiTableView->item(index,1)->foreground().color();
}

bool ZROIWidget::isRoiChecked(int index) const
{
  return getRoiItem(index)->checkState() == Qt::Checked;
}

QString ZROIWidget::getRoiName(int index) const
{
  return getRoiItem(index)->text();
}

#endif

/*
void ZROIWidget::filterRoi()
{
  filterRoi(m_filterEdit->text());
}
*/

/*
ZMesh* ZROIWidget::getRoiMesh(int index) const
{
  if (index >= 0 && size_t(index) < m_roiList.size()) {
    return  m_roiList[size_t(index)]->getMesh();
  }

  return nullptr;
}
*/

void ZROIWidget::filterRoi()
{
  if (m_filterRegexCheckBox->isChecked()) {
    m_roiItemModelProxy->setFilterRegExp(m_filterEdit->text());
  } else {
    m_roiItemModelProxy->setFilterFixedString(m_filterEdit->text());
  }
}
#if 0
void ZROIWidget::filterRoi(const QString &filterString)
{
  m_roiItemModelProxy->setFilterFixedString(filterString);
  /*
  for(int i = 0; i < m_roiTableView->rowCount(); ++i) {
    if (filterString.isEmpty()) {
      m_roiTableView->setRowHidden(i, false);
    } else {
      m_roiTableView->setRowHidden(i, !getRoiName(i).contains(filterString));
    }
  }
  */
}
#endif

/*
void ZROIWidget::updateSelectedROIs()
{
  // render selected ROIs
  if (m_window != NULL) {
    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<m_roiTableView->rowCount(); i++) {
      if(isRoiChecked(i)) {
        ZMesh *sourceMesh = getRoiMesh(i);
        if (sourceMesh) {
          QColor color = getRoiColor(i);
          ZMesh *mesh = new ZMesh(*sourceMesh);
          mesh->setColor(color);
          mesh->pushObjectColor();
          m_window->getDocument()->addObject(mesh);
        }
      }
    }
    m_window->getDocument()->blockSignals(false);
    m_window->getDocument()->notify3DCubeModified();
  }
}
*/

#if 0
void ZROIWidget::updateROIs()
{
  if (m_window == NULL) {
    return;
  }

  // render selected ROIs
//    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<m_roiTableView->rowCount(); i++) {
      QTableWidgetItem *it = m_roiTableView->item(i, 0);

      auto roiMesh = m_roiList[i];

#ifdef _DEBUG_
      std::cout << "Updating ROI: " << roiMesh->getName() << std::endl;
#endif

      ZStackObject *obj = m_window->getDocument()->getObject(
            ZStackObject::EType::MESH, roiMesh->getSourceName());
      ZMesh *mesh = dynamic_cast<ZMesh*>(obj);

      if(it->checkState()==Qt::Checked) { //visible meshes
        QColor color = m_roiTableView->item(i,1)->foreground().color();

        bool addingMesh = false;
        bool meshUpdated = false;
//        bool updatingColor = colorModified[i];
//        colorModified[i] = false;

        if (mesh == NULL) {
          mesh = roiMesh->makeMesh();
//          mesh = new ZMesh(*m_loadedROIs.at(i));
//          mesh->setSource(m_roiSourceList[i]);
          addingMesh = true;
        }

        if (mesh != NULL) {
          if (mesh->getColor() != color || addingMesh) {
            mesh->setColor(color);
            mesh->pushObjectColor();
            meshUpdated = true;
          }
          if (!mesh->isVisible()) {
            mesh->setVisible(true);
            meshUpdated = true;
          }
          if (addingMesh) {
            m_window->getDocument()->addObject(mesh);
          } else if (meshUpdated) {
#ifdef _DEBUG_
            std::cout << "ROI color: " << mesh->getColor().red() << std::endl;
#endif
            m_window->getDocument()->bufferObjectModified(mesh);
          }
        }
      } else {
        if (mesh != NULL) {
          if (mesh->isVisible()) {
            mesh->setVisible(false);
            m_window->getDocument()->bufferObjectModified(mesh);
          }
        }
      }
    }
    m_window->getDocument()->processObjectModified();
//    m_window->getDocument()->blockSignals(false);
//    m_window->getDocument()->notify3DCubeModified();
}


void ZROIWidget::updateROISelections(int row, int column)
{
    QTableWidgetItem *item = m_roiTableView->item(row, 0);

    if(column==0)
    {
        //
        if(item->checkState()==Qt::Checked)
        {
            item->setCheckState(Qt::Unchecked);
        }
        else
        {
            item->setCheckState(Qt::Checked);
        }

        //
        updateROIs();

    }
    else if(column==1)
    {
        // edit color
    }
}
#endif

void ZROIWidget::handleRoiTableDoubleClick(const QModelIndex &index)
{
  if (index.column() == ZRoiItemModel::COLUMN_COLOR) {
    updateRoiColor(index.row());
  } if (index.column() == ZRoiItemModel::COLUMN_NAME) {
    QString name = m_roiItemModel->getName(getSourceModelIndex(index.row()));
    emit locatingRoiMesh(name);
  }
}

void ZROIWidget::updateRoiColor(int row)
{
//  QColor newcolor = QColorDialog::getColor(item->foreground().color());
  int index = getSourceModelIndex(row);
  QColor newColor = QColorDialog::getColor(m_roiItemModel->getColor(index));
  if (newColor.isValid()) {
    m_roiItemModel->setColor(index, newColor);
  }
}

#if 0
void ZROIWidget::updateROIColors(int row, int column)
{
    QTableWidgetItem *item = m_roiTableView->item(row, 1);

    if(column==1)
    {
        //QColor newcolor = QColorDialog::getColor(item->backgroundColor());
        //item->setBackgroundColor(newcolor);
        QColor newcolor = QColorDialog::getColor(item->foreground().color());
        if( !newcolor.isValid() )
        {
           return;
        }

        //
        QBrush brush(newcolor);
        item->setForeground(brush);

        m_colorModified[row] = true;

        //
        updateROIs();

    }
    else if(column==0)
    {
        // edit selection
    }
}
#endif

void ZROIWidget::updateOpacityLabel(double v)
{
  m_opacityLabel->setText(tr(" Opacity: %1").arg(v, 0, 'f', 2, '0'));
}

/*
void ZROIWidget::updateROIRendering(QTableWidgetItem* item)
{
    ZOUT(LTRACE(), 5)<<"to render ROI: "<<item->text()<<item->checkState();


    ZOUT(LTRACE(), 5)<<m_roiTableView->selectedItems();
}
*/

void ZROIWidget::updateSlider(int v)
{
    double alpha = double( v / 100.0 );
    updateOpacityLabel(alpha);
//    m_opacityLabel->setText(tr(" Opacity: %1").arg(alpha));

    if(m_window)
    {
      m_window->setOpacityQuietly(neutu3d::ERendererLayer::ROI, alpha);
//        m_window->get()->setOpacity(alpha);
    }
}

void ZROIWidget::updateOpacity(double v)
{
    int opacityVal = 100*v;
    m_opacitySlider->setValue(opacityVal);
    updateOpacityLabel(v);
//    m_opacityLabel->setText(tr(" Opacity: %1").arg(v));
}



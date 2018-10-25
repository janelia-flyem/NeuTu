
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

#include "neutubeconfig.h"
#include "flyem/zflyemmisc.h"
#include "zstackobjectsourcefactory.h"
#include "z3dwindow.h"
#include "z3dsurfacefilter.h"
#include "zroiwidget.h"
//#include "flyem/zflyemproofdoc.h"
//
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

//
ZROIWidget::ZROIWidget(QWidget *parent) : QDockWidget(parent)
{
    m_roiList.clear();
    m_defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    m_roiList.clear();
    m_defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

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

void ZROIWidget::loadROIs(Z3DWindow *window,
//                         const ZDvidInfo &dvidInfo,
                         std::vector<std::string> roiList,
                         std::vector<ZSharedPointer<ZMesh> > loadedROIs,
                         std::vector<std::string> roiSourceList)
{
    //
    m_window = window;
//    m_dvidInfo =  dvidInfo;
    loadROIs(roiList, loadedROIs, roiSourceList);
}

void ZROIWidget::loadROIs(std::vector<std::string> roiList,
                          std::vector<ZSharedPointer<ZMesh> > loadedROIs,
                          std::vector<std::string> roiSourceList)
{
  m_roiList = roiList;
  m_loadedROIs = loadedROIs;
  m_roiSourceList = roiSourceList;

  if (m_window != NULL) {
    m_colorModified.resize(m_roiList.size(), false);
    makeGUI();
  }
}

void ZROIWidget::setCheckStatus(int row, bool on)
{
  if (row >= 0 && row < (int) m_checkStatus.size()) {
    m_checkStatus[row] = on;

    QTableWidgetItem *item = tw_ROIs->item(row, 0);
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

void ZROIWidget::makeGUI()
{
    if(m_roiList.empty())
        return;

    //
    m_selectAll = new QCheckBox("Select All");
    m_selectAll->setChecked(false);

    //
    double alpha = m_window->getFilter(neutube3d::LAYER_ROI)->opacity();
 //   double alpha = m_window->getSurfaceFilter()->getOpacity();
    m_opacityLabel = new QLabel(tr(" Opacity: %1").arg(alpha));
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
    tw_ROIs = new QTableWidget(0, 2);
    tw_ROIs->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("ROI Name") << tr("Color");
    tw_ROIs->setHorizontalHeaderLabels(labels);

#ifdef _QT5_
    tw_ROIs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
#else
    tw_ROIs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
#endif
//    tw_ROIs->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    tw_ROIs->verticalHeader()->hide();
    tw_ROIs->setShowGrid(false);

    //
    //QBrush brush(defaultColor);
    size_t roiCount = m_roiList.size();
    for (std::size_t i = 0; i < roiCount; ++i)
    {
        QTableWidgetItem *roiNameItem = new QTableWidgetItem(QString(m_roiList[i].c_str()));
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

        int row = tw_ROIs->rowCount();
        tw_ROIs->insertRow(row);
        tw_ROIs->setItem(row, 0, roiNameItem);
        tw_ROIs->setItem(row, 1, colorItem);

        bool checked = false;
        m_checkStatus.push_back(checked);
    }

    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *controlLayout = new QHBoxLayout();

    controlLayout->addWidget(m_selectAll);
    controlLayout->addWidget(horizontalGroupBox);

    layout->addLayout(controlLayout);
    layout->addWidget(tw_ROIs);

    QGroupBox *group = new QGroupBox();
    group->setLayout(layout);

#ifdef _DEBUG_
    std::cout << "Set ROI widget" << std::endl;
#endif

    this->setWidget(group);

    //
    //connect(tw_ROIs, SIGNAL(cellClicked(int,int)), this, SLOT(updateROISelections(int,int)));
    connect(tw_ROIs, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(updateROIColors(int,int)));
    connect(tw_ROIs, SIGNAL(clicked(QModelIndex)),
            this, SLOT(updateROISelections(QModelIndex)));
    connect(m_selectAll, SIGNAL(clicked()), this, SLOT(updateSelection()));
    connect(m_opacitySlider,SIGNAL(valueChanged(int)),this,SLOT(updateSlider(int)));
    connect(m_window->getFilter(neutube3d::LAYER_ROI), SIGNAL(opacityChanged(double)),
            this,SLOT(updateOpacity(double)));
}


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

int ZROIWidget::getDsIntv() const
{
  return 0;
//  return m_dsIntvWidget->value();
}

void ZROIWidget::updateSelection()
{
    if(m_selectAll->isChecked())
    {
        for(int i=0; i<tw_ROIs->rowCount(); i++)
        {
            QTableWidgetItem *item = tw_ROIs->item(i, 0);
            item->setCheckState(Qt::Checked);
        }
    }
    else
    {
        for(int i=0; i<tw_ROIs->rowCount(); i++)
        {
            QTableWidgetItem *item = tw_ROIs->item(i, 0);
            item->setCheckState(Qt::Unchecked);
        }
    }

    updateROIs();
}

void ZROIWidget::updateSelectedROIs()
{
  // render selected ROIs
  if (m_window != NULL) {
    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<tw_ROIs->rowCount(); i++)
    {
      QTableWidgetItem *it = tw_ROIs->item(i, 0);

      if(it->checkState()==Qt::Checked)
      {
        QColor color = tw_ROIs->item(i,1)->foreground().color();
        ZMesh *mesh = new ZMesh(*m_loadedROIs.at(i));
        mesh->setColor(color);
        mesh->pushObjectColor();
        m_window->getDocument()->addObject(mesh);

//        ZCubeArray *cubes = ZFlyEmMisc::MakeRoiCube(
//              m_loadedROIs.at(i), color, getDsIntv());
//        cubes->setSource(m_roiSourceList[i]);
//        cubes->setColor(color);
//        m_window->getDocument()->addObject(cubes, true);
      }
    }
    m_window->getDocument()->blockSignals(false);
    m_window->getDocument()->notify3DCubeModified();
  }
}


void ZROIWidget::updateROIs()
{
  if (m_window == NULL) {
    return;
  }

  // render selected ROIs
//    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<tw_ROIs->rowCount(); i++) {
      QTableWidgetItem *it = tw_ROIs->item(i, 0);

#ifdef _DEBUG_
      std::cout << "Updating ROI: " << m_roiList[i] << std::endl;
#endif

      ZStackObject *obj = m_window->getDocument()->getObject(
            ZStackObject::TYPE_MESH, m_roiSourceList[i]);
      ZMesh *mesh = dynamic_cast<ZMesh*>(obj);

      if(it->checkState()==Qt::Checked) { //visible meshes
        QColor color = tw_ROIs->item(i,1)->foreground().color();

        bool addingMesh = false;
        bool meshUpdated = false;
//        bool updatingColor = colorModified[i];
//        colorModified[i] = false;

        if (mesh == NULL) {
          mesh = new ZMesh(*m_loadedROIs.at(i));
          mesh->setSource(m_roiSourceList[i]);
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
    QTableWidgetItem *item = tw_ROIs->item(row, 0);

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

void ZROIWidget::updateROIColors(int row, int column)
{
    QTableWidgetItem *item = tw_ROIs->item(row, 1);

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

void ZROIWidget::updateROIRendering(QTableWidgetItem* item)
{
    ZOUT(LTRACE(), 5)<<"to render ROI: "<<item->text()<<item->checkState();


    ZOUT(LTRACE(), 5)<<tw_ROIs->selectedItems();
}

void ZROIWidget::updateSlider(int v)
{
    double alpha = double( v / 100.0 );
    m_opacityLabel->setText(tr(" Opacity: %1").arg(alpha));

    if(m_window)
    {
      m_window->setOpacityQuietly(neutube3d::LAYER_ROI, alpha);
//        m_window->get()->setOpacity(alpha);
    }
}

void ZROIWidget::updateOpacity(double v)
{
    int opacityVal = 100*v;
    m_opacitySlider->setValue(opacityVal);
    m_opacityLabel->setText(tr(" Opacity: %1").arg(v));
}



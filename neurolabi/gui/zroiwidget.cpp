
#include <vector>
#include <string>
#include <iostream>

#include <QtGui>
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

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
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    m_roiList.clear();
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);

    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    //
    m_objmodel = new ZROIObjsModel(this);
}

ZROIWidget::~ZROIWidget()
{

}

void ZROIWidget::closeEvent(QCloseEvent * /*event*/)
{
    emit toBeClosed();
}

void ZROIWidget::getROIs(Z3DWindow *window,
                         ZDvidInfo &dvidInfo,
                         std::vector<std::string> roiList,
                         std::vector<ZObject3dScan> loadedROIs,
                         std::vector<std::string> roiSourceList)
{
    //
    m_window = window;
    m_dvidInfo =  dvidInfo;
    m_roiList = roiList;
    m_loadedROIs = loadedROIs;
    m_roiSourceList = roiSourceList;

    //
    size_t N = m_roiList.size();

    //
    if (window != NULL)
    {
        for(size_t i=0; i<N; i++)
            colorModified.push_back(false);

        //
        window->setROIs(N);

        //
        makeGUI();

    }
}

void ZROIWidget::makeGUI()
{
    if(m_roiList.empty())
        return;

    //
    selectAll = new QCheckBox("Select All");
    selectAll->setChecked(false);

    //
    double alpha = m_window->getSurfaceFilter()->getOpacity();
    l_opacity = new QLabel(tr(" Opacity: %1").arg(alpha));
    s_opacity = new QSlider(Qt::Horizontal);
    s_opacity->setRange(0,100);
    s_opacity->setValue(alpha*100);

    //
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(selectAll);
    hLayout->addStretch();
    hLayout->addWidget(l_opacity);
    hLayout->addWidget(s_opacity);

    QGroupBox *horizontalGroupBox = new QGroupBox();
    horizontalGroupBox->setLayout(hLayout);

    //
    tw_ROIs = new QTableWidget(0, 2);
    tw_ROIs->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("ROI Name") << tr("Color");
    tw_ROIs->setHorizontalHeaderLabels(labels);
    tw_ROIs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
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

    //
    //this->setWidget(tw_ROIs);
    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *controlLayout = new QHBoxLayout();


    m_dsIntvWidget = new QSpinBox(this);
    m_dsIntvWidget->setRange(0, 5);
    m_dsIntvWidget->setValue(1);
    controlLayout->addWidget(new QLabel(tr("Downsample"), this));
    controlLayout->addWidget(m_dsIntvWidget);

    m_updateButton = new QPushButton();
    m_updateButton->setText(tr("Update"));
    controlLayout->addWidget(m_updateButton);

    layout->addLayout(controlLayout);
    layout->addWidget(selectAll);
    layout->addWidget(horizontalGroupBox);

    layout->addWidget(tw_ROIs);

    QGroupBox *group = new QGroupBox();
    group->setLayout(layout);

    this->setWidget(group);

    //
    //connect(tw_ROIs, SIGNAL(cellClicked(int,int)), this, SLOT(updateROISelections(int,int)));
    connect(tw_ROIs, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(updateROIColors(int,int)));

    connect(tw_ROIs, SIGNAL(clicked(QModelIndex)), this, SLOT(updateROISelections(QModelIndex)));


    connect(selectAll, SIGNAL(clicked()), this, SLOT(updateSelection()));
    connect(m_updateButton, SIGNAL(clicked()), this, SLOT(updateSelectedROIs()));

    //connect(tw_ROIs, SIGNAL(itemActivated(QTableWidgetItem *)), this, SLOT(updateROIRendering(QTableWidgetItem*)));

    connect(s_opacity,SIGNAL(valueChanged(int)),this,SLOT(updateSlider(int)));
    connect(m_window->getSurfaceFilter(),SIGNAL(opacityValueChanged(double)),this,SLOT(updateOpacity(double)));
}


void ZROIWidget::updateROISelections(QModelIndex idx)
{
    int row = idx.row();
    int col = idx.column();

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
}

int ZROIWidget::getDsIntv() const
{
  return m_dsIntvWidget->value();
}

void ZROIWidget::updateSelection()
{
    if(selectAll->isChecked())
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

        ZCubeArray *cubes = ZFlyEmMisc::MakeRoiCube(
              m_loadedROIs.at(i), m_dvidInfo, color, getDsIntv());
        cubes->setSource(m_roiSourceList[i]);
        cubes->setColor(color);
        m_window->getDocument()->addObject(cubes, true);
        m_window->getSurfaceFilter()->invalidateRenderer(cubes->getSource());
      }
    }
    m_window->getDocument()->blockSignals(false);
    m_window->update3DCubeDisplay();
  }
}


void ZROIWidget::updateROIs()
{
  // render selected ROIs
  ZDvidReader reader;
  ZDvidTarget target(m_dvidInfo.getDvidAddress(),
                     m_dvidInfo.getDvidUuid(),
                     m_dvidInfo.getDvidPort());

  if (reader.open(target)) {
    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<tw_ROIs->rowCount(); i++)
    {
      QTableWidgetItem *it = tw_ROIs->item(i, 0);

      if(it->checkState()==Qt::Checked)
      {
        if(m_window != NULL)
        {
          QColor color = tw_ROIs->item(i,1)->foreground().color();

          if(m_window->getDocument()->hasObject(
               ZStackObject::TYPE_3D_CUBE, m_roiSourceList[i])==false)
          {
            ZIntPoint blockSize = reader.readRoiBlockSize(m_roiList[i]);

            ZDvidInfo dvidInfo = m_dvidInfo;
            dvidInfo.setBlockSize(blockSize.getX(), blockSize.getY(),
                                  blockSize.getZ());

            ZCubeArray *cubes = ZFlyEmMisc::MakeRoiCube(
                  m_loadedROIs.at(i), dvidInfo, color, getDsIntv());
            cubes->setSource(m_roiSourceList[i]);
            cubes->setColor(color);
            m_window->getDocument()->addObject(cubes, true);
          }
          else
          {
            if(colorModified[i] == true)
            {
              qDebug()<<"~~~color is changed"<<color;

              colorModified[i] = false;
              m_window->getDocument()->getObject(ZStackObject::TYPE_3D_CUBE, m_roiSourceList[i])->setColor(color);
            }
            m_window->getDocument()->setVisible(ZStackObject::TYPE_3D_CUBE, m_roiSourceList[i], true);
          }
        }
      }
      else
      {
        if(m_window != NULL)
        {
          m_window->getDocument()->setVisible(ZStackObject::TYPE_3D_CUBE, m_roiSourceList[i], false);
        }
      }
    }
    m_window->getDocument()->blockSignals(false);
    m_window->update3DCubeDisplay();
  }
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

        colorModified[row] = true;

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
    l_opacity->setText(tr(" Opacity: %1").arg(alpha));

    if(m_window)
    {
        m_window->getSurfaceFilter()->setOpacity(alpha);
    }
}

void ZROIWidget::updateOpacity(double v)
{
    int opacityVal = 100*v;
    s_opacity->setValue(opacityVal);
    l_opacity->setText(tr(" Opacity: %1").arg(v));
}



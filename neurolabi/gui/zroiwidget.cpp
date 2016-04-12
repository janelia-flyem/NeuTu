
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

#include "flyem/zflyemmisc.h"
#include "zstackobjectsourcefactory.h"
#include "z3dwindow.h"

#include "zroiwidget.h"

ZROIWidget::ZROIWidget(QWidget *parent) : QDockWidget(parent)
{
    m_roiList.clear();
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);
}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    m_roiList.clear();
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);
}

ZROIWidget::~ZROIWidget()
{

}

void ZROIWidget::closeEvent(QCloseEvent * event)
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
        QBrush brush(zcolor.getColor(i));

        QTableWidgetItem *colorItem = new QTableWidgetItem(tr("@COLOR"));
        colorItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //colorItem->setBackgroundColor(defaultColor);
        colorItem->setForeground(brush);

        int row = tw_ROIs->rowCount();
        tw_ROIs->insertRow(row);
        tw_ROIs->setItem(row, 0, roiNameItem);
        tw_ROIs->setItem(row, 1, colorItem);
    }

    //
    this->setWidget(tw_ROIs);

    //
    connect(tw_ROIs, SIGNAL(cellClicked(int,int)), this, SLOT(updateROISelections(int,int)));
    connect(tw_ROIs, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(updateROIColors(int,int)));

    //connect(tw_ROIs, SIGNAL(itemActivated(QTableWidgetItem *)), this, SLOT(updateROIRendering(QTableWidgetItem*)));
}

void ZROIWidget::updateROIs()
{
    // render selected ROIs
    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<tw_ROIs->rowCount(); i++)
    {
        QTableWidgetItem *it = tw_ROIs->item(i, 0);

        if(it->checkState()==Qt::Checked)
        {
            if(m_window != NULL)
            {
                QColor color = tw_ROIs->item(i,1)->foreground().color();

                if(m_window->getDocument()->hasObject(ZStackObject::TYPE_3D_CUBE, m_roiSourceList[i] )==false)
                {
                    ZCubeArray *cubes = ZFlyEmMisc::MakeRoiCube(m_loadedROIs.at(i), m_dvidInfo, color);
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
    qDebug()<<"to render ROI: "<<item->text()<<item->checkState();


    qDebug()<<tw_ROIs->selectedItems();
}




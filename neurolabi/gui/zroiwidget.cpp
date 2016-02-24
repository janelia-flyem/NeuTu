
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
    roiList.clear();
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);
}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    roiList.clear();
    defaultColor.setRgbF(0.5f, 0.25f, 0.25f, 1.0f);
}

ZROIWidget::~ZROIWidget()
{

}

void ZROIWidget::getROIs(Z3DWindow *window, const ZDvidInfo &dvidInfo, const ZDvidTarget &dvidTarget)
{
    m_window = window;
    m_dvidInfo =  dvidInfo;
    m_dvidTarget = dvidTarget;

    //
    if (window != NULL)
    {
        //if (!dvidTarget.getRoiName().empty()){
        if (reader.open(dvidTarget))
        {
            ZJsonObject meta = reader.readInfo();

            //
            ZJsonValue datains = meta.value("DataInstances");

            if(datains.isObject())
            {
                ZJsonObject insList(datains.getData(), true);
                std::vector<std::string> keys = insList.getAllKey();

                for(std::size_t i=0; i<keys.size(); i++)
                {
                    std::size_t found = keys.at(i).find("roi");

                    if(found!=std::string::npos)
                    {
                        qDebug()<<" rois: "<<keys.at(i);


                        ZObject3dScan roi = reader.readRoi(keys.at(i));
                        if(!roi.isEmpty())
                        {
                            roiList.push_back(keys.at(i));
                        }
                    }
                }
                qDebug()<<"~~~~~~~~~~~~ test dvid roi reading ~~~~~~~~~~~~~"<<roiList.size();
            }

            //
            makeGUI();
        }
        //} // dvid target is not empty
    } // window is not null
}

void ZROIWidget::makeGUI()
{
    if(roiList.empty())
        return;

    //
    tw_ROIs = new QTableWidget(0, 2);
    tw_ROIs->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList labels;
    labels << tr("ROI name") << tr("Color");
    tw_ROIs->setHorizontalHeaderLabels(labels);
    tw_ROIs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    tw_ROIs->verticalHeader()->hide();
    tw_ROIs->setShowGrid(false);

    //
    for (std::size_t i = 0; i < roiList.size(); ++i)
    {
        QTableWidgetItem *roiNameItem = new QTableWidgetItem(QString(roiList[i].c_str()));
        roiNameItem->setFlags(roiNameItem->flags() ^ Qt::ItemIsEditable);
        roiNameItem->setCheckState(Qt::Unchecked);

        QTableWidgetItem *colorItem = new QTableWidgetItem(tr("red"));
        colorItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //colorItem->setFlags(colorItem->flags() ^ Qt::ItemIsEditable);

        int row = tw_ROIs->rowCount();
        tw_ROIs->insertRow(row);
        tw_ROIs->setItem(row, 0, roiNameItem);
        tw_ROIs->setItem(row, 1, colorItem);
    }

    //
    this->setWidget(tw_ROIs);

    //
    //connect(tw_ROIs, SIGNAL(cellActivated(int,int)), this, SLOT(updateROIRendering(int,int)));
    connect(tw_ROIs, SIGNAL(cellClicked(int,int)), this, SLOT(updateROIRendering(int,int)));
    //connect(tw_ROIs, SIGNAL(itemActivated(QTableWidgetItem *)), this, SLOT(updateROIRendering(QTableWidgetItem*)));
}

void ZROIWidget::updateROIRendering(int row, int column)
{
    QTableWidgetItem *item = tw_ROIs->item(row, 0);

    qDebug()<<"render ROI: "<<item->text()<<item->checkState()<<column;

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

        // render selected ROIs
        ZCubeArray *cubes = new ZCubeArray;
        for(int i=0; i<tw_ROIs->rowCount(); i++)
        {
            QTableWidgetItem *it = tw_ROIs->item(i, 0);
            if(it->checkState()==Qt::Checked)
            {
                ZObject3dScan roi = reader.readRoi(it->text().toStdString());
                if (!roi.isEmpty())
                {
                    ZFlyEmMisc::MakeRoiCube(cubes, roi, m_dvidInfo, defaultColor);
                    //cubes->setSource( ZStackObjectSourceFactory::MakeFlyEmRoiSource( it->text().toStdString() ));
                }
            }
        }
        if(m_window != NULL)
        {
            m_window->getDocument()->addObject(cubes, true);
        }
    }
    else if(column==1)
    {
        // edit color
    }

}

void ZROIWidget::updateROIRendering(QTableWidgetItem* item)
{
    qDebug()<<"to render ROI: "<<item->text()<<item->checkState();


    qDebug()<<tw_ROIs->selectedItems();
}




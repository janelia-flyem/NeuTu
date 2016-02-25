
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
                            loadedROIs.push_back(roi);

                            std::string source = ZStackObjectSourceFactory::MakeFlyEmRoiSource( keys.at(i) );
                            roiSourceList.push_back(source);
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
    labels << tr("ROI Name") << tr("Color");
    tw_ROIs->setHorizontalHeaderLabels(labels);
    tw_ROIs->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    tw_ROIs->verticalHeader()->hide();
    tw_ROIs->setShowGrid(false);

    //
    //QBrush brush(defaultColor);
    for (std::size_t i = 0; i < roiList.size(); ++i)
    {
        QTableWidgetItem *roiNameItem = new QTableWidgetItem(QString(roiList[i].c_str()));
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
    //ZCubeArray *cubes = new ZCubeArray;

    qDebug()<<"~~~~ debug updateROIs";

    m_window->getDocument()->blockSignals(true);
    for(int i=0; i<tw_ROIs->rowCount(); i++)
    {
        QTableWidgetItem *it = tw_ROIs->item(i, 0);

        if(it->checkState()==Qt::Checked)
        {
            if(m_window != NULL)
            {
                if(m_window->getDocument()->hasObject(ZStackObject::TYPE_3D_CUBE, it->text().toStdString() )==false)
                {
                    QColor color = tw_ROIs->item(i,1)->foreground().color();

                    qDebug()<<"~~~~ makeroicube ...";

                    //
                    ZCubeArray *cubes = ZFlyEmMisc::MakeRoiCube(loadedROIs.at(i), m_dvidInfo, color);

                    qDebug()<<"~~~~ makeroicube ... done";

                    cubes->setSource(roiSourceList[i]);

                    qDebug()<<"~~~~ setSource ... done" << roiSourceList[i];

                    m_window->getDocument()->addObject(cubes, true);

                    qDebug()<<"~~~~ addObject ... done";
                }
                else
                {
                    m_window->getDocument()->setVisible(ZStackObject::TYPE_3D_CUBE, roiSourceList[i], true);
                }
            }
        }
        else
        {
            if(m_window != NULL)
            {
                m_window->getDocument()->setVisible(ZStackObject::TYPE_3D_CUBE, roiSourceList[i], false);
            }
        }
    }
    m_window->getDocument()->blockSignals(false);
    m_window->update3DCubeDisplay();

    qDebug()<<"~~~~ debug updateROIs done";

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




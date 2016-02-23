#include <QtGui>

#include "zroiwidget.h"

ZROIWidget::ZROIWidget(QWidget *parent) : QDockWidget(parent)
{

}

ZROIWidget::ZROIWidget(const QString & title, QWidget * parent, Qt::WindowFlags flags) : QDockWidget(title, parent, flags)
{
    roiList.clear();
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
        if (!dvidTarget.getRoiName().empty())
        {
            ZDvidReader reader;
            if (reader.open(dvidTarget))
            {
                ZJsonObject meta = reader.readInfo();

                //
                ZJsonValue datains = meta.value("DataInstances");

                if(datains.isObject())
                {
                    ZJsonObject insList(datains.getData(), true);
                    std::vector<std::string> keys = insList.getAllKey();

                    for(int i=0; i<keys.size(); i++)
                    {
                        std::size_t found = keys.at(i).find("roi");

                        if(found!=std::string::npos)
                        {
                            qDebug()<<" rois: "<<keys.at(i);

                            roiList.push_back(keys.at(i));
                        }
                    }
                    qDebug()<<"~~~~~~~~~~~~ test dvid roi reading ~~~~~~~~~~~~~"<<roiList.size();
                }

                //       ZObject3dScan roi = reader.readRoi(dvidTarget.getRoiName());
                //        if (!roi.isEmpty()) {
                //          ZCubeArray *cubes = MakeRoiCube(roi, dvidInfo);
                //          cubes->setSource(
                //                ZStackObjectSourceFactory::MakeFlyEmRoiSource(
                //                  dvidTarget.getRoiName()));
                //          window->getDocument()->addObject(cubes, true);
                //        }

                //
                makeGUI();
            }
        } // dvid target is not empty
    } // window is not null
}

void ZROIWidget::makeGUI()
{
    qDebug()<<"~~~~~~~~~makeGUI";

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
    qDebug()<<"~~~~~ "<<roiList.size();

    //
    for (int i = 0; i < roiList.size(); ++i)
    {
        QTableWidgetItem *roiNameItem = new QTableWidgetItem(QString(roiList[i].c_str()));
        //roiNameItem->setFlags(roiNameItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *colorItem = new QTableWidgetItem(tr("red"));
        //colorItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //colorItem->setFlags(colorItem->flags() ^ Qt::ItemIsEditable);

        int row = tw_ROIs->rowCount();
        tw_ROIs->insertRow(row);
        tw_ROIs->setItem(row, 0, roiNameItem);
        tw_ROIs->setItem(row, 1, colorItem);

        qDebug()<<"insert one row"<<i;
    }

    //
    this->setWidget(tw_ROIs);


    //
    connect(tw_ROIs, SIGNAL(cellActivated(int,int)), this, SLOT(updateROIRendering(int,int)));
}

void ZROIWidget::updateROIRendering(int row, int /* column */)
{
    QTableWidgetItem *item = tw_ROIs->item(row, 0);

    qDebug()<<"render ROI: "<<item->text();
}




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
                    qDebug()<<"~~~~~~~~~~~~ test dvid roi reading ~~~~~~~~~~~~~";
                }

                //       ZObject3dScan roi = reader.readRoi(dvidTarget.getRoiName());
                //        if (!roi.isEmpty()) {
                //          ZCubeArray *cubes = MakeRoiCube(roi, dvidInfo);
                //          cubes->setSource(
                //                ZStackObjectSourceFactory::MakeFlyEmRoiSource(
                //                  dvidTarget.getRoiName()));
                //          window->getDocument()->addObject(cubes, true);
                //        }
            }
        }
    }
}

void ZROIWidget::makeGUI()
{

}




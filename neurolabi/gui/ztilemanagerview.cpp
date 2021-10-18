#include "ztilemanagerview.h"
#include <QScrollBar>
#include <QDebug>

//#include "zstackdrawable.h"
#include "ztilemanager.h"
#include "zswctree.h"
#include "zpainter.h"
#include "data3d/displayconfig.h"

ZTileManagerView::ZTileManagerView(QWidget *parent) :
  QGraphicsView(parent)
{
    swcVisible = true;
}

void ZTileManagerView::paintEvent(QPaintEvent *event)
{
   QGraphicsView::paintEvent(event);

   if (swcVisible) {
//     ZPainter viewPainter(this->viewport());

     QScrollBar* vSc;
     vSc = this->verticalScrollBar();
     QScrollBar* hSc;
     hSc = this->horizontalScrollBar();

#if 0
     QTransform transform;
     transform.translate(-hSc->value(),-vSc->value());

     transform.scale(this->transform().m11() ,
                     this->transform().m22() );




#if 0
     QTransform transform = this->transform();
     float scale = 0.1;
//     float scale = getParentWindow()->getScaleFactor();

     QScrollBar* vSc;
     vSc = this->verticalScrollBar();
     QScrollBar* hSc;
     hSc = this->horizontalScrollBar();
     transform.translate(hSc->value(),vSc->value());

     transform.scale(scale, scale);
#endif
     viewPainter.setTransform(transform);
#endif
     neutu::data3d::DisplayConfig config;
     config.setViewCanvasTransform(
           -hSc->value(), -vSc->value(), this->transform().m11());
     config.setSliceMode(neutu::data3d::EDisplaySliceMode::PROJECTION);
     config.setStyle(neutu::data3d::EDisplayStyle::BOUNDARY);

     QPainter painter(this->viewport());
     QList<ZSwcTree*> swcList = getParentWindow()->getDocument()->getSwcList();
     foreach (ZSwcTree *swc, swcList ) {
       if (swc != NULL) {
         swc->display(&painter, config);
//         swc->display(viewPainter,-1, ZStackObject::EDisplayStyle::BOUNDARY, neutu::EAxis::Z);
       }
     }
   }
}


/*
void ZTileManagerView::slotTest()
{
    qDebug() << "Slot triggered: ZTileManagerView::slotTest()";
    update();
}
*/

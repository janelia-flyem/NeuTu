#include "ztilemanagerview.h"
#include "zstackdrawable.h"
#include "ztilemanager.h"
#include <QScrollBar>
#include <QDebug>

ZTileManagerView::ZTileManagerView(QWidget *parent) :
  QGraphicsView(parent)
{
    swcVisible = true;
}

void ZTileManagerView::paintEvent(QPaintEvent *event)
{
   QGraphicsView::paintEvent(event);

   if (swcVisible) {
     ZPainter viewPainter(this->viewport());


     QTransform transform;

     QScrollBar* vSc;
     vSc = this->verticalScrollBar();
     QScrollBar* hSc;
     hSc = this->horizontalScrollBar();
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

     QList<ZSwcTree*> swcList = getParentWindow()->getDocument()->getSwcList();
     foreach (ZSwcTree *swc, swcList ) {
       if (swc != NULL) {
         swc->display(viewPainter,-1, ZStackObject::BOUNDARY, NeuTube::Z_AXIS);
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

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
       float scale = getParentWindow()->getScaleFactor();

       QScrollBar* vSc;
       vSc = this->verticalScrollBar();
       QScrollBar* hSc;
       hSc = this->horizontalScrollBar();
       transform.translate(-hSc->value(),-vSc->value());

       transform.scale(scale, scale);
       viewPainter.setTransform(transform);

       QList<ZSwcTree*> swcList = getParentWindow()->getDocument()->getSwcList();
       foreach (ZSwcTree *swc, swcList ) {
           if (swc != NULL) {
               swc->display(viewPainter,-1, ZStackObject::SOLID);
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

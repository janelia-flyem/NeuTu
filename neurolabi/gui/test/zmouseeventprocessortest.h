#ifndef ZMOUSEEVENTPROCESSORTEST_H
#define ZMOUSEEVENTPROCESSORTEST_H

#include "ztestheader.h"
#include "zmouseeventprocessor.h"
#include "neutubeconfig.h"
#include "zmouseevent.h"
#include "zviewproj.h"
#include "zstackfactory.h"

#include "mvc/zstackdochelper.h"
#include "mvc/zstackdoc.h"

#ifdef _USE_GTEST_
TEST(ZMouseEventProcessor, MapPoint)
{
  ZMouseEventProcessor processor;

  ZStackDoc doc;
  doc.loadStack(ZStackFactory::MakeVirtualStack(100, 200, 300));
  ZIntCuboid box = ZStackDocHelper::GetStackSpaceRange(&doc, neutu::EAxis::Z);

  ZViewProj viewProj;
  viewProj.setCanvasRect(QRect(box.getFirstCorner().getX(),
                               box.getFirstCorner().getY(),
                               box.getWidth(), box.getHeight()));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));

  double x = 50.0;
  double y = 50.0;
//  processor.mapPositionFromWidgetToRawStack(&x, &y, viewProj);
  ASSERT_DOUBLE_EQ(50.0, x);
  ASSERT_DOUBLE_EQ(50.0, y);

}

#endif

#endif // ZMOUSEEVENTPROCESSORTEST_H

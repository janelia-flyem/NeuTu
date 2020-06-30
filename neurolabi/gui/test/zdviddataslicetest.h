#ifndef ZDVIDDATASLICETEST_H
#define ZDVIDDATASLICETEST_H

#include "ztestheader.h"
#include "dvid/zdviddataslicehelper.h"
#include "zstackviewparam.h"

#ifdef _USE_GTEST_

TEST(ZDvidDataSliceHelper, Basic)
{
  ZDvidDataSliceHelper helper(ZDvidData::ERole::GRAYSCALE);

  ZStackViewParam viewParam;
  viewParam.setCutCenter(ZIntPoint(7500, 6000, 3619));
  viewParam.setSize(512, 512, neutu::data3d::ESpace::MODEL);
//  viewParam.setCanvasRect(QRect(0, 0, 10000, 20000));
//  viewParam.setWidgetRect(QRect(0, 0, 100, 100));
//  viewParam.setViewPort(QRect(7286, 5630, 512, 512), 3619);

  helper.setViewParam(viewParam);
//  qDebug() << helper.getViewPort();
  ASSERT_EQ(512, helper.getWidth());
  ASSERT_EQ(512, helper.getHeight());

  ZStackViewParam vp2 = viewParam;
  vp2.closeViewPort();
  helper.setViewParam(vp2);
  ASSERT_TRUE(helper.actualContainedIn(
                viewParam, helper.getZoom(), helper.getCenterCutWidth(),
                helper.getCenterCutHeight(), helper.usingCenterCut()));

  viewParam.closeViewPort();
  ASSERT_FALSE(helper.actualContainedIn(
                 viewParam, helper.getZoom(), helper.getCenterCutWidth(),
                 helper.getCenterCutHeight(), helper.usingCenterCut()));

  viewParam.openViewPort();
  ASSERT_TRUE(helper.actualContainedIn(
                viewParam, helper.getZoom(), helper.getCenterCutWidth(),
                helper.getCenterCutHeight(), helper.usingCenterCut()));

  viewParam.moveCutDepth(1);
  ASSERT_FALSE(helper.actualContainedIn(
                 viewParam, helper.getZoom(), helper.getCenterCutWidth(),
                 helper.getCenterCutHeight(), helper.usingCenterCut()));
}

TEST(ZDvidDataSliceHelper, Resolution)
{
  ZDvidDataSliceHelper helper(ZDvidData::ERole::MULTISCALE_2D);
  helper.setMaxZoom(5);

  ZStackViewParam viewParam;
  viewParam.setCutCenter(ZIntPoint(7500, 6000, 3619));
  viewParam.setSize(512, 512, neutu::data3d::ESpace::MODEL);
//  viewParam.setCanvasRect(QRect(0, 0, 10000, 20000));
//  viewParam.setWidgetRect(QRect(0, 0, 100, 100));
//  viewParam.setViewPort(QRect(7286, 5630, 512, 512), 3619);

  helper.setViewParam(viewParam);
  helper.setZoom(1);
  helper.syncActualQuality();
  ASSERT_TRUE(helper.isResolutionReached());
  ASSERT_TRUE(helper.needHighResUpdate());

  helper.setCenterCut(128, 128);
  helper.setZoom(1);
  helper.useCenterCut(true);
  helper.syncActualQuality();
  ASSERT_TRUE(helper.isResolutionReached());
  ASSERT_TRUE(helper.needHighResUpdate());

  helper.setZoom(0);
  ASSERT_FALSE(helper.isResolutionReached());

  helper.setZoom(2);
  ASSERT_TRUE(helper.isResolutionReached());

  helper.setZoom(1);
  helper.setCenterCut(64, 64);
  ASSERT_TRUE(helper.isResolutionReached());

  helper.setCenterCut(300, 300);
  ASSERT_FALSE(helper.isResolutionReached());

  helper.setCenterCut(64, 300);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setCenterCut(512, 512);
  ASSERT_TRUE(!helper.isResolutionReached());

  helper.setActualQuality(1, 0, 0, false);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setZoom(0);
  ASSERT_TRUE(!helper.isResolutionReached());

  helper.setZoom(1);
  helper.useCenterCut(false);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setActualQuality(1, 0, 0, true);
  ASSERT_TRUE(!helper.isResolutionReached());

  helper.setActualQuality(1, 0, 0, false);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setActualQuality(2, 512, 512, true);
  ASSERT_TRUE(!helper.isResolutionReached());

  helper.setActualQuality(1, 512, 512, true);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setActualQuality(1, 256, 512, true);
  ASSERT_TRUE(!helper.isResolutionReached());

  helper.useCenterCut(true);
  helper.setZoom(5); //max zoom; centercut should be automatically off
  helper.setActualQuality(5, 0, 0, true);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setActualQuality(6, 0, 0, true);
  ASSERT_FALSE(!helper.isResolutionReached());

  helper.setZoom(6);
  ASSERT_FALSE(!helper.isResolutionReached());

  ZStackViewParam vp = helper.getViewParam();
  vp.closeViewPort();
  helper.setViewParam(vp);
  ASSERT_TRUE(!helper.isResolutionReached());
}



#endif


#endif // ZDVIDDATASLICETEST_H

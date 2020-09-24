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
  ASSERT_EQ(512, helper.getWidth(0));
  ASSERT_EQ(512, helper.getHeight(0));

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
  helper.syncActualQuality(0);
  ASSERT_TRUE(helper.isResolutionReached(0));
  ASSERT_TRUE(helper.needHighResUpdate(0));

  helper.setCenterCut(128, 128);
  helper.setZoom(1);
  helper.useCenterCut(true);
  helper.syncActualQuality(0);
  ASSERT_TRUE(helper.isResolutionReached(0));
  ASSERT_TRUE(helper.needHighResUpdate(0));

  helper.setZoom(0);
  ASSERT_FALSE(helper.isResolutionReached(0));

  helper.setZoom(2);
  ASSERT_TRUE(helper.isResolutionReached(0));

  helper.setZoom(1);
  helper.setCenterCut(64, 64);
  ASSERT_TRUE(helper.isResolutionReached(0));

  helper.setCenterCut(300, 300);
  ASSERT_FALSE(helper.isResolutionReached(0));

  helper.setCenterCut(64, 300);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setCenterCut(512, 512);
  ASSERT_TRUE(!helper.isResolutionReached(0));

  helper.setActualQuality(1, 0, 0, false, 0);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setZoom(0);
  ASSERT_TRUE(!helper.isResolutionReached(0));

  helper.setZoom(1);
  helper.useCenterCut(false);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setActualQuality(1, 0, 0, true, 0);
  ASSERT_TRUE(!helper.isResolutionReached(0));

  helper.setActualQuality(1, 0, 0, false, 0);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setActualQuality(2, 512, 512, true, 0);
  ASSERT_TRUE(!helper.isResolutionReached(0));

  helper.setActualQuality(1, 512, 512, true, 0);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setActualQuality(1, 256, 512, true, 0);
  ASSERT_TRUE(!helper.isResolutionReached(0));

  helper.useCenterCut(true);
  helper.setZoom(5); //max zoom; centercut should be automatically off
  helper.setActualQuality(5, 0, 0, true, 0);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setActualQuality(6, 0, 0, true, 0);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  helper.setZoom(6);
  ASSERT_FALSE(!helper.isResolutionReached(0));

  ZStackViewParam vp = helper.getViewParam(0);
  vp.closeViewPort();
  helper.setViewParam(vp);
  ASSERT_TRUE(!helper.isResolutionReached(0));
}

TEST(ZDvidDataSliceHelper, ViewParam)
{
  ZDvidDataSliceHelper helper(ZDvidData::ERole::GRAYSCALE);
  helper.setZoom(1);

  ASSERT_FALSE(helper.getViewParam(0).isValid());

  ZStackViewParam viewParam;
  viewParam.setCutCenter(ZIntPoint(7500, 6000, 3619));
  viewParam.setSize(512, 512, neutu::data3d::ESpace::MODEL);
  viewParam.setViewId(1);

  helper.setViewParam(viewParam);

  ASSERT_FALSE(helper.getViewParam(0).isValid());
  ASSERT_TRUE(helper.getViewParam(1).isValid());

  helper.closeViewPort(0);
}


#endif


#endif // ZDVIDDATASLICETEST_H

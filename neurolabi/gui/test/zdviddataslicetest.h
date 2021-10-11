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

  helper.setViewParamLastUpdate(viewParam);
//  qDebug() << helper.getViewPort();
  ASSERT_EQ(512, helper.getWidth(0));
  ASSERT_EQ(512, helper.getHeight(0));

  ZStackViewParam vp2 = viewParam;
  vp2.closeViewPort();
  helper.setViewParamLastUpdate(vp2);
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

  int width = 1024;
  int height = 1024;
  helper.validateSize(&width, &height);
  ASSERT_GE(helper.getMaxWidth(), width);
  ASSERT_GE(helper.getMaxHeight(), height);
//  std::cout << width << " " << height << std::endl;
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

  helper.setViewParamLastUpdate(viewParam);
  helper.setZoom(1);
  helper.syncActualQuality(0);
  ASSERT_TRUE(helper.isResolutionReached(0));
  ASSERT_TRUE(helper.highResUpdateNeeded(0));

  helper.setCenterCut(128, 128);
  helper.setZoom(1);
  helper.useCenterCut(true);
  helper.syncActualQuality(0);
  ASSERT_TRUE(helper.isResolutionReached(0));
  ASSERT_TRUE(helper.highResUpdateNeeded(0));

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

  ZStackViewParam vp = helper.getViewParamLastUpdate(0);
  vp.closeViewPort();
  helper.setViewParamLastUpdate(vp);
  ASSERT_TRUE(!helper.isResolutionReached(0));
}

TEST(ZDvidDataSliceHelper, ViewParam)
{
  ZDvidDataSliceHelper helper(ZDvidData::ERole::GRAYSCALE);
  helper.setZoom(1);

  ASSERT_FALSE(helper.getViewParamLastUpdate(0).isValid());

  {
    ZStackViewParam viewParam;
    viewParam.setSliceAxis(neutu::EAxis::Y);
    viewParam.setCutCenter(ZIntPoint(7500, 6000, 3619));
    viewParam.setSize(512, 512, neutu::data3d::ESpace::MODEL);
    viewParam.setViewId(1);
    helper.setViewParamLastUpdate(viewParam);
  }

  ASSERT_FALSE(helper.getViewParamLastUpdate(0).isValid());
  ASSERT_TRUE(helper.getViewParamLastUpdate(1).isValid());

  helper.closeViewPort(0);

  ASSERT_FALSE(helper.getViewParamLastUpdate(2).isValid());
  {
    ZStackViewParam viewParam;
    viewParam.setSliceAxis(neutu::EAxis::X);
    viewParam.setCutCenter(ZIntPoint(7500, 6000, 3619));
    viewParam.setSize(512, 512, neutu::data3d::ESpace::MODEL);
    viewParam.setViewId(2);
    helper.setViewParamLastUpdate(viewParam);
  }
  ASSERT_TRUE(helper.getViewParamLastUpdate(2).isValid());

  ASSERT_EQ(neutu::EAxis::Y, helper.getSliceAxis(1));
  ASSERT_EQ(neutu::EAxis::X, helper.getSliceAxis(2));

  ZStackViewParam vp;
  vp.setSliceAxis(neutu::EAxis::Y);
  vp.setCutCenter(ZIntPoint(7500, 6000, 3619));
  vp.setSize(512, 512, neutu::data3d::ESpace::MODEL);

  ASSERT_TRUE(helper.actualContainedIn(vp, 0, 0, 0, false));
  vp.setViewId(1);
  ASSERT_FALSE(helper.actualContainedIn(vp, 0, 0, 0, false));
  vp.setViewId(2);
  ASSERT_FALSE(helper.actualContainedIn(vp, 0, 0, 0, false));
  vp.setViewId(3);
  ASSERT_TRUE(helper.actualContainedIn(vp, 0, 0, 0, false));

  helper.setActualQuality(1, 256, 256, true, 1);
  helper.setActualQuality(2, 256, 256, true, 2);

  ASSERT_EQ(1, helper.getActualZoom(1));
  ASSERT_EQ(2, helper.getActualScale(1));

  ASSERT_EQ(2, helper.getActualZoom(2));
  ASSERT_EQ(4, helper.getActualScale(2));

  ASSERT_TRUE(helper.getViewParamLastUpdate(1).isValid());
  helper.invalidateViewParam(1);
  ASSERT_FALSE(helper.getViewParamLastUpdate(1).isValid());

  ASSERT_TRUE(helper.getViewParamLastUpdate(2).isValid());
  helper.invalidateAllViewParam();
  ASSERT_FALSE(helper.getViewParamLastUpdate(2).isValid());
}

TEST(ZDvidDataSliceHelper, hit)
{
  ZDvidDataSliceHelper helper(ZDvidData::ERole::GRAYSCALE);

  ASSERT_FALSE(helper.hit(100, 200, 300, 0));

  ZStackViewParam param;
  param.setSliceAxis(neutu::EAxis::Z);
  param.setCutCenter({100, 200, 300});
  param.setSize(256, 512, neutu::data3d::ESpace::MODEL);
  param.setViewId(1);
  helper.setViewParamLastUpdate(param);

  ASSERT_FALSE(helper.hit(100, 200, 300, 0));
  ASSERT_TRUE(helper.hit(100, 200, 300, 1));
  ASSERT_FALSE(helper.hit(100, 200, 300, 2));

  param.setSliceAxis(neutu::EAxis::Y);
  param.setCutCenter({100, 200, 300});
  param.setSize(256, 512, neutu::data3d::ESpace::MODEL);
  param.setViewId(2);
  helper.setViewParamLastUpdate(param);
  ASSERT_TRUE(helper.hit(150, 200, 350, 2));
}

#endif


#endif // ZDVIDDATASLICETEST_H

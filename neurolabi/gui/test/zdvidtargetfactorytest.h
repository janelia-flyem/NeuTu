#ifndef ZDVIDTARGETFACTORYTEST_H
#define ZDVIDTARGETFACTORYTEST_H

#include "ztestheader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidtargetfactory.h"
#include "dvid/zdvidtargetbuilder.h"

#ifdef _USE_GTEST_

TEST(ZDvidTargetBuilder, Basic)
{
  {
    ZDvidTarget target = ZDvidTargetBuilder().on("http://test.com:8000");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().on("test.com:8000");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().on("https://test.com:8000");
    ASSERT_EQ("https", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().on("test.com");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(-1, target.getPort());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().
        on("http://test.com:8000").withUuid("1234");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
    ASSERT_EQ("1234", target.getUuid());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().
        on("http://test.com:8000").
        withUuid("1234").
        withSegmentation("segmentation");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
    ASSERT_EQ("1234", target.getUuid());
    ASSERT_EQ("segmentation", target.getSegmentationName());
  }

  {
    ZDvidTarget target = ZDvidTargetBuilder().
        on("http://test.com:8000").
        roi().add("roi1").set("mainROI").
        main().
        withUuid("1234").
        withSegmentation("segmentation");
    ASSERT_EQ("http", target.getScheme());
    ASSERT_EQ("test.com", target.getAddress());
    ASSERT_EQ(8000, target.getPort());
    ASSERT_EQ("1234", target.getUuid());
    ASSERT_EQ("segmentation", target.getSegmentationName());
    ASSERT_EQ("mainROI", target.getRoiName());
    ASSERT_EQ(1, int(target.getRoiList().size()));
  }

}

TEST(ZDvidTargetFactory, Basic)
{
  {
    ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(
          "http:test.org:9100:1234:body_test:graytest");
    ASSERT_EQ("http:test.org:9100:1234:body_test", target.getSourceString());
    ASSERT_EQ("http:test.org:9100:1234::graytest",
              target.getGrayscaleSourceString());

  }

  {
    ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(
          "http://test.org:9100?uuid=5678&segmentation=body_test&grayscale=graytest");

    ASSERT_EQ("http:test.org:9100:5678:body_test", target.getSourceString());
    ASSERT_EQ("graytest", target.getGrayScaleName());
    ASSERT_EQ("http:test.org:9100:5678::graytest",
              target.getGrayscaleSourceString());
  }

  {
    ZDvidTarget target = ZDvidTargetFactory::MakeFromJsonSpec(
          "{"
          "  \"address\": \"127.0.0.1\","
          "  \"port\": 1600,"
          "  \"uuid\": \"c315\","
          "  \"segmentation\": \"segtest\""
          "}");
    ASSERT_EQ("http:127.0.0.1:1600:c315", target.getSourceString());
    ASSERT_EQ("segtest", target.getSegmentationName());
  }



}

#endif


#endif // ZDVIDTARGETFACTORYTEST_H

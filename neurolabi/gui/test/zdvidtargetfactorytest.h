#ifndef ZDVIDTARGETFACTORYTEST_H
#define ZDVIDTARGETFACTORYTEST_H

#include "ztestheader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidtargetfactory.h"

#ifdef _USE_GTEST_

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

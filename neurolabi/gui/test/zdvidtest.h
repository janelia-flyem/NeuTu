#ifndef ZDVIDTEST_H
#define ZDVIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdviddef.h"
#include "dvid/zdvidinfo.h"
//#include "dvid/zdvidbuffer.h"
#include "dvid/zdvidtarget.h"
#include "dialogs/zdviddialog.h"
#include "zstring.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdviddata.h"
#include "zdvidutil.h"
#include "dvid/zdvidnode.h"
#include "geometry/zintcuboid.h"
#include "zobject3dscan.h"

#ifdef _USE_GTEST_

TEST(ZDvidTest, Util)
{
  ASSERT_TRUE(dvid::IsUuidMatched("12345", "123"));
  ASSERT_TRUE(dvid::IsUuidMatched("aad345", "aad"));
  ASSERT_TRUE(dvid::IsUuidMatched("123", "123fwrq424q"));
  ASSERT_FALSE(dvid::IsUuidMatched("", "123"));
  ASSERT_FALSE(dvid::IsUuidMatched("12345", ""));
  ASSERT_FALSE(dvid::IsUuidMatched("12345", "12346"));
  ASSERT_FALSE(dvid::IsUuidMatched("234", "12346"));

  ZDvidTarget target = dvid::MakeTargetFromUrlSpec(
        "http://emdata4.int.janelia.org:8900?uuid=52a1&"
        "segmentation=seg&grayscale=gs&admintoken=mytoken");

  ASSERT_EQ("http:emdata4.int.janelia.org:8900:52a1:seg", target.getSourceString());
  ASSERT_EQ("seg", target.getSegmentationName());
  ASSERT_EQ("gs", target.getGrayScaleName());
  ASSERT_EQ("mytoken", target.getAdminToken());
}


TEST(ZDvidTest, Reader)
{
  ZDvidReader reader;
  ASSERT_FALSE(reader.open("foo:9001"));
  ASSERT_FALSE(reader.open("", "uuid", 1));
  ASSERT_FALSE(reader.open("server", "", 1));

#if 0
//  ASSERT_TRUE(reader.open("http://emdata2.int.janelia.org:9000:2ad1"));
  if (reader.open("http://emdata2.int.janelia.org:9000:2ad1")) {
    std::cout << "Connected to " << reader.getDvidTarget().getAddressWithPort()
              << std::endl;
    ASSERT_TRUE(reader.good());
    ASSERT_TRUE(reader.isReady());

    ZDvidReader reader2;
    ASSERT_FALSE(reader2.open(reader.getDvidTarget().getAddress().c_str(), "",
                              reader.getDvidTarget().getPort()));
    ASSERT_FALSE(reader2.open("", reader.getDvidTarget().getUuid().c_str(),
                              reader.getDvidTarget().getPort()));
  }
#endif
}

TEST(ZDvidTest, DataType)
{
  ASSERT_EQ(dvid::EDataType::LABELBLK, dvid::GetDataType("labelblk"));
  ASSERT_EQ(dvid::EDataType::ANNOTATION, dvid::GetDataType("annotation"));
  ASSERT_EQ(dvid::EDataType::IMAGETILE, dvid::GetDataType("imagetile"));
  ASSERT_EQ(dvid::EDataType::KEYVALUE, dvid::GetDataType("keyvalue"));
  ASSERT_EQ(dvid::EDataType::LABELGRAPH, dvid::GetDataType("labelgraph"));
  ASSERT_EQ(dvid::EDataType::LABELSZ, dvid::GetDataType("labelsz"));
  ASSERT_EQ(dvid::EDataType::LABELVOL, dvid::GetDataType("labelvol"));
  ASSERT_EQ(dvid::EDataType::ROI, dvid::GetDataType("roi"));
  ASSERT_EQ(dvid::EDataType::UINT8BLK, dvid::GetDataType("uint8blk"));

  /*
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "e2f0", 7000);
  target.setSegmentationName("segmentation2");
  ZDvidReader reader;
  if (reader.open(target)) {
    reader.syncBodyLabelName();
    ASSERT_EQ("segmentation-labelvol", reader.getDvidTarget().getBodyLabelName());
  }
  */
}

#endif

#endif // ZDVIDTEST_H

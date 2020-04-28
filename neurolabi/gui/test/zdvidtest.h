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

TEST(ZDvidTest, ZDvidInfo)
{
  ZDvidInfo info;
  info.print();
  std::cout << std::endl;
  /*
  const char* ZDvidInfo::m_minPointKey = "MinPoint";
  const char* ZDvidInfo::m_maxPointKey = "MaxPoint";
  const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
  const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
  const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
*/
  info.setFromJsonString("{ "
                         " \"MinPoint\": [1, 2, 3], "
                         " \"BlockSize\": [16, 32, 64],"
                         " \"MaxPoint\": [1000, 2000, 3000],"
                         " \"MinIndex\": [4, 5, 6],"
                         " \"MaxIndex\": [1000, 2000, 3000]"
                         "}");
  info.print();

  ASSERT_EQ(1, info.getStartCoordinates().getX());
  ASSERT_EQ(2, info.getStartCoordinates().getY());
  ASSERT_EQ(3, info.getStartCoordinates().getZ());

  ASSERT_EQ(1000, info.getEndCoordinates().getX());
  ASSERT_EQ(2000, info.getEndCoordinates().getY());
  ASSERT_EQ(3000, info.getEndCoordinates().getZ());

  ASSERT_EQ(16, info.getBlockSize().getX());
  ASSERT_EQ(32, info.getBlockSize().getY());
  ASSERT_EQ(64, info.getBlockSize().getZ());

  ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockIndex(0, 0, 0));
  ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-1, -1, -1));
  ASSERT_EQ(ZIntPoint(1, 1, 1), info.getBlockIndex(16, 32, 64));
  ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-16, -32, -64));
  ASSERT_EQ(ZIntPoint(-2, -2, -2), info.getBlockIndex(-17, -33, -65));
  ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockCoord(0, 0, 0));
  ASSERT_EQ(ZIntPoint(16, 32, 64), info.getBlockCoord(1, 1, 1));
  ZIntCuboid box = info.getBlockBox(1, 2, 3);
  ASSERT_EQ(ZIntPoint(16, 64, 192), box.getMinCorner());
  ASSERT_EQ(16, box.getWidth());
  ASSERT_EQ(32, box.getHeight());
  ASSERT_EQ(64, box.getDepth());

  ASSERT_TRUE(info.isValidBlockIndex(ZIntPoint(100, 20, 30)));
  ASSERT_FALSE(info.isValidBlockIndex(ZIntPoint(100, 20, 3)));

  ZObject3dScan obj;
  obj.addSegment(-1, -2, -1, 10);
  ZObject3dScan obj2 = info.getBlockIndex(obj);
  ASSERT_EQ(2, int(obj2.getVoxelNumber()));
  ASSERT_TRUE(obj2.contains(-1, -1, -1));
  ASSERT_TRUE(obj2.contains(0, -1, -1));
}

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

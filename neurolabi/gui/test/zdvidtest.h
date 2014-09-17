#ifndef ZDVIDTEST_H
#define ZDVIDTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidbuffer.h"
#include "dvid/zdvidtarget.h"
#include "zdviddialog.h"
#include "zstring.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidurl.h"

#ifdef _USE_GTEST_

TEST(ZDvidTest, ZDvidInfo)
{
  ZDvidInfo info;
  info.print();

  /*
  const char* ZDvidInfo::m_minPointKey = "MinPoint";
  const char* ZDvidInfo::m_maxPointKey = "MaxPoint";
  const char* ZDvidInfo::m_blockSizeKey = "BlockSize";
  const char* ZDvidInfo::m_voxelSizeKey = "VoxelSize";
  const char* ZDvidInfo::m_blockMinIndexKey = "MinIndex";
*/
  info.setFromJsonString("{ \"MinPoint\": [1, 2, 3]}");
  info.print();

  ASSERT_EQ(1, info.getStartCoordinates().getX());
  ASSERT_EQ(2, info.getStartCoordinates().getY());
  ASSERT_EQ(3, info.getStartCoordinates().getZ());
}

TEST(ZDvidTest, ZDvidUrl)
{
  ZDvidTarget target("emdata.janelia.org", "bf1");
  //const std::vector<ZDvidTarget> &dvidRepo =
  //    NeutubeConfig::getInstance().getFlyEmConfig().getDvidRepo();
  ZDvidUrl dvidUrl(target);
  std::cout << dvidUrl.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons",
            dvidUrl.getSkeletonUrl());
}

#endif

#endif // ZDVIDTEST_H

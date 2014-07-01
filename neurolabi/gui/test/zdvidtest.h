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

#ifdef _USE_GTEST_

TEST(ZDvidInfo, basic)
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
}

TEST(ZDvidTarget, basic)
{
  ZDvidTarget target = GET_FLYEM_CONFIG.getDvidTarget();
  target.print();

  target.set("http:emdata1.int.janelia.org:-1:b42");
  target.print();

  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));
  //if (dlg.exec()) {}

  target = dlg.getDvidTarget();
  target.print();
}

TEST(ZDvidReader, basic)
{
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target = dlg.getDvidTarget();
  target.print();

  ZDvidReader reader;
  ASSERT_FALSE(reader.open(""));

  ASSERT_TRUE(reader.open(target));
  qDebug() << reader.readInfo("skeletons");

  QString info = reader.readInfo("superpixels");
  qDebug() << info;
}

#endif

#endif // ZDVIDTEST_H

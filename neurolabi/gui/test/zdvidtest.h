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

#endif

#endif // ZDVIDTEST_H

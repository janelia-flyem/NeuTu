#ifndef ZDVIDIOTEST_H
#define ZDVIDIOTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

#ifdef _USE_GTEST_

TEST(ZDvidTarget, basic)
{
#ifdef _FLYEM_
  ZDvidTarget target;
  ASSERT_FALSE(target.isValid());

  target.set("http:emdata1.int.janelia.org:9000:b42");
  ASSERT_TRUE(target.isValid());

  target = GET_FLYEM_CONFIG.getDvidTarget();
  target.print();

  target.set("http:emdata1.int.janelia.org:-1:b42");
  target.print();

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));
  //if (dlg.exec()) {}

  target = dlg.getDvidTarget();
  target.print();
#endif

#endif
}

TEST(ZDvidReader, basic)
{
#ifdef _FLYEM_2
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidReader reader;

  ZDvidTarget target;
  ASSERT_FALSE(reader.open(target));

  target = dlg.getDvidTarget();
  target.print();

  ASSERT_FALSE(reader.open(""));

  ASSERT_TRUE(reader.open(target));
  qDebug() << reader.readInfo("skeletons");

  QString info = reader.readInfo("superpixels");
  qDebug() << info;
#endif
}


#endif

#endif // ZDVIDIOTEST_H

#ifndef ZDVIDIOTEST_H
#define ZDVIDIOTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

#if defined(_USE_GTEST_) && defined(_FLYEM_)

TEST(ZDvidTarget, basic)
{
  ZDvidTarget target;
  ASSERT_FALSE(target.isValid());

  target.setFromSourceString("http:emdata1.int.janelia.org:9000:b42");
  ASSERT_TRUE(target.isValid());

  target = GET_FLYEM_CONFIG.getDvidTarget();
  target.print();

  target.setFromSourceString("http:emdata1.int.janelia.org:-1:b42");
  target.print();

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));
  //if (dlg.exec()) {}

  target = dlg.getDvidTarget();
  target.print();
#endif
}

TEST(ZDvidReader, basic)
{
  ZFlyEmConfig config;

  config.loadConfig(ZString::fullPath(GET_APPLICATION_DIR,
                                      "json", "", "flyem_config.json"));


  ZDvidReader reader;

  ZDvidTarget target;
  ASSERT_FALSE(reader.open(target));

  target = config.getDvidRepo().front();
  target.print();

  ASSERT_FALSE(reader.open(""));

  ASSERT_TRUE(reader.open(target));
  //qDebug() << reader.readInfo("skeletons");

#if 0
  QString info = reader.readInfo("superpixels");
  qDebug() << info;
#endif
}

TEST(ZDvidWriter, basic)
{
  ZDvidTarget dvidTarget("emdata2.int.janelia.org", "628");
  ZDvidWriter writer;
  ASSERT_TRUE(writer.open(dvidTarget));

  //writer.createData("keyvalue", "skeletons");
}

#endif

#endif // ZDVIDIOTEST_H

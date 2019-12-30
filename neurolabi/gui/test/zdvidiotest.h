#ifndef ZDVIDIOTEST_H
#define ZDVIDIOTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidtarget.h"

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

  config.setConfigPath(ZString::fullPath(GET_CONFIG_DIR,
                                         "json", "", "flyem_config.json"));
  config.useDefaultConfig(false);
  config.loadConfig();

  {
    ZDvidReader reader;

    ZDvidTarget target;
    ASSERT_FALSE(reader.open(target));

    target = config.getDvidRepo().front();
    target.print();

    ASSERT_FALSE(reader.open(""));
  }

  {
    ZDvidReader reader;
    if (reader.open("127.0.0.1", "4280", 1600)) {
      ASSERT_TRUE(reader.isReady());

      ASSERT_EQ(dvid::ENodeStatus::LOCKED, reader.getNodeStatus())
          << "Actual status: " + std::to_string(neutu::EnumValue(reader.getNodeStatus()));
      ASSERT_TRUE(reader.hasData("grayscale"));
      ASSERT_TRUE(reader.hasData("segmentation"));

      ZDvidReader reader2 = reader;
      ASSERT_TRUE(reader2.isReady());
      ASSERT_EQ(dvid::ENodeStatus::LOCKED, reader2.getNodeStatus());
      ASSERT_TRUE(reader2.hasData("grayscale"));
      ASSERT_TRUE(reader2.hasData("segmentation"));

    } else {
      std::cout << "Testing skipped because 127.0.0.1:3600:4280 is not available"
                << std::endl;
    }
  }

  {
    ZDvidReader reader;
    ZDvidTarget target("127.0.0.1", "c315", 1600);
    target.setGrayScaleName("grayscale");
    if (reader.open(target)) {
      ASSERT_TRUE(reader.isReady());

      ASSERT_EQ(dvid::ENodeStatus::NORMAL, reader.getNodeStatus());
      ASSERT_TRUE(reader.hasData("grayscale"));
      ASSERT_TRUE(reader.hasData("segmentation"));

      ZDvidReader reader2 = reader;
      ASSERT_TRUE(reader2.isReady());
      ASSERT_EQ(dvid::ENodeStatus::NORMAL, reader2.getNodeStatus());
      ASSERT_TRUE(reader2.hasData("grayscale"));
      ASSERT_TRUE(reader2.hasData("segmentation"));

      ZDvidInfo info = reader2.readGrayScaleInfo();
      ASSERT_TRUE(info.isValid());
    }
  }


//  ASSERT_TRUE(reader.open(target));
  //qDebug() << reader.readInfo("skeletons");

#if 0
  QString info = reader.readInfo("superpixels");
  qDebug() << info;
#endif
}

TEST(ZDvidWriter, basic)
{
//  ZDvidTarget dvidTarget("emdata2.int.janelia.org", "628");
  ZDvidWriter writer;
  ASSERT_EQ("",
            writer.getDvidTarget().getAddressWithPort());
//  ASSERT_TRUE(writer.open(dvidTarget));

  //writer.createData("keyvalue", "skeletons");
}

#endif

#endif // ZDVIDIOTEST_H

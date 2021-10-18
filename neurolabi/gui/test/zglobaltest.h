#ifndef ZGLOBALTEST_H
#define ZGLOBALTEST_H

#include "ztestheader.h"

#include "zglobal.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidreader.h"

#ifdef _USE_GTEST_

TEST(ZGlobal, DVID)
{
  ZDvidTarget target;
  std::string url =
      "mock://emdata2.int.janelia.org:9000/api/node/3456/branches/key/master";
  target.setFromUrl_deprecated(url);

  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader(target);
  reader->getDvidTarget().print();

  ASSERT_TRUE(reader != NULL);

  ASSERT_EQ(reader, ZGlobal::GetInstance().getDvidReader(target));

  ASSERT_EQ(reader, ZGlobal::GetDvidReaderFromUrl(url));

  reader = ZGlobal::GetInstance().getDvidReaderFromUrl(
        "mock://emdata3.int.janelia.org:9100/api/node/1234/body_test/sparsevol/123");
  ASSERT_EQ("body_test", reader->getDvidTarget().getBodyLabelName());

  reader = ZGlobal::GetDvidReader(
        "mock:emdata3.int.janelia.org:9100:1234:body_test");
  ASSERT_EQ("body_test", reader->getDvidTarget().getBodyLabelName());

  ZDvidReader *reader2 = ZGlobal::GetDvidReader(
        "mock:emdata3.int.janelia.org:9100:1234:body_test", "test");
  ASSERT_EQ("body_test", reader2->getDvidTarget().getBodyLabelName());
  ASSERT_NE(reader, reader2);

  ASSERT_EQ(reader2, ZGlobal::GetDvidReader(
              "mock:emdata3.int.janelia.org:9100:1234:body_test", "test"));
  ASSERT_NE(reader2, ZGlobal::GetDvidReader(
              "mock:emdata3.int.janelia.org:9100:1234:body_test", "test2"));
}

#endif

#endif // ZGLOBALTEST_H

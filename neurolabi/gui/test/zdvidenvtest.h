#ifndef ZDVIDENVTEST_H
#define ZDVIDENVTEST_H

#include <iostream>

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "dvid/zdvidenv.h"
#include "zjsonobject.h"

#ifdef _USE_GTEST_
TEST(ZDvidEnv, Basic)
{
  ZDvidEnv env;

  ASSERT_FALSE(env.isValid());

  ZDvidTarget target;
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_test/dvid_setting.json");
  target.loadJsonObject(obj);

  env.set(target);

  std::cout << env.toJsonObject().dumpString(2) << std::endl;


  ASSERT_TRUE(env.isValid());
}

#endif


#endif // ZDVIDENVTEST_H

#ifndef MAINTEST_H
#define MAINTEST_H

#include "ztestheader.h"

#include "main.h"

#ifdef _USE_GTEST_

TEST(main, util)
{
//  std::cout << get_main_config_path("test") << std::endl;
  ASSERT_EQ("test/json/config.json", get_main_config_path("test"));
}

#endif

#endif // MAINTEST_H

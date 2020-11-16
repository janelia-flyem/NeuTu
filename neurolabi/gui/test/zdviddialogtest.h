#ifndef ZDVIDDIALOGTEST_H
#define ZDVIDDIALOGTEST_H

#include <iostream>

#include "ztestheader.h"
#include "dialogs/zdviddialog.h"

#ifdef _USE_GTEST_

TEST(ZDvidDialog, basic)
{
  ZDvidDialog dlg;
  dlg.forEachTarget([](const ZDvidTarget &target) {
    if (target.getName() == ZDvidDialog::CUSTOM_NAME) {
      ASSERT_FALSE(target.isValid());
    } else {
      ASSERT_TRUE(target.isValid()) << target.toJsonObject().dumpString(2);
    }
//    std::cout << target.getName() << " "
//              << target.getSourceString() << std::endl;
  });

  ZDvidDialog::PrivateTest test(&dlg);
  ASSERT_FALSE(test.hasNameConflict("1234567"));
  ASSERT_TRUE(test.hasNameConflict("MB_Test"));

  test.setSever(0);
  ASSERT_EQ(ZDvidDialog::CUSTOM_NAME, dlg.getDvidTarget().getName());
}

#endif

#endif // ZDVIDDIALOGTEST_H

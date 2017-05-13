#ifndef Z3DFILTERSETTINGTEST_H
#define Z3DFILTERSETTINGTEST_H

#include "ztestheader.h"
#include "z3dfiltersetting.h"
#include "zjsonobject.h"

#ifdef _USE_GTEST_

TEST(Z3DFilterSetting, basic)
{
  Z3DFilterSetting setting;
  setting.setVisible(false);

  ASSERT_FALSE(setting.isVisible());

  ZJsonObject obj;
  obj.setEntry(Z3DFilterSetting::FRONT_KEY, false);
  obj.setEntry(Z3DFilterSetting::VISIBLE_KEY, true);
  obj.setEntry(Z3DFilterSetting::COLOR_MODE_KEY, "Intrinsic");
  obj.setEntry(Z3DFilterSetting::SHAPE_MODE_KEY, "Normal");
  obj.setEntry(Z3DFilterSetting::SIZE_SCALE_KEY, 2.0);

  setting.load(obj);

  ASSERT_FALSE(setting.isFront());
  ASSERT_TRUE(setting.isVisible());
  ASSERT_EQ("Intrinsic", setting.getColorMode());
  ASSERT_EQ("Normal", setting.getShapeMode());
  ASSERT_EQ(2.0, setting.getSizeScale());
}
#endif


#endif // Z3DFILTERSETTINGTEST_H

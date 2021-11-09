#ifndef Z3DWINDOWTEST_H
#define Z3DWINDOWTEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "z3dwindow.h"
#include "mvc/zstackdoc.h"

TEST(Z3DWindow, Make)
{
  ZStackDoc *doc = new ZStackDoc;
  Z3DWindow *win = Z3DWindow::Make(doc, nullptr, Z3DView::EInitMode::TEST);
  ASSERT_NE(nullptr, win);
}

#endif

#endif // Z3DWINDOWTEST_H

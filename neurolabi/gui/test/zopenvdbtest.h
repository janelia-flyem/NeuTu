#ifndef ZOPENVDBTEST_H
#define ZOPENVDBTEST_H

#include "ztestheader.h"
#include "zopenvdbobject.h"
#include "neutubeconfig.h"

#ifdef _USE_GTEST_

#if defined(_USE_OPENVDB_)

TEST(ZOpenVdbObject, basic)
{
  ZOpenVdbObject obj;
  obj.setValue(0, 0, 0, 1);
  obj.setValue(1, 1, 1, 1);
  for (size_t i = 0; i < 10000000; ++i) {
    obj.setValue(i, i, i, i);
  }

  obj.print();

}
#endif

#endif


#endif // ZOPENVDBTEST_H

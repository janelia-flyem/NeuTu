#ifndef ZFLYEM_H
#define ZFLYEM_H

#include "tz_utilities.h"
#include "tz_cdefs.h"

namespace FlyEm {

enum EDataSet {
  DATA_TEM, DATA_FIB19, DATA_FIB25, DATA_FIB25_7C, DATA_UNKOWN
};

typedef tz_uint64 TBodyLabel;
}

#endif // ZFLYEM_H

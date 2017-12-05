#ifndef ZFLYEM_H
#define ZFLYEM_H

#include "tz_utilities.h"
#include "tz_cdefs.h"

namespace flyem {

enum EDataSet {
  DATA_TEM, DATA_FIB19, DATA_FIB25, DATA_FIB25_7C, DATA_UNKOWN
};

typedef tz_uint64 TBodyLabel;

const static char *GROUP_BOX_STYLE =
    "QGroupBox {  border: 1px solid lightgray; border-radius: 3px; margin-top: 2ex;}"
    "QGroupBox::title {subcontrol-origin: margin; subcontrol-position: top left; padding: 0 3px;}";
}

#endif // ZFLYEM_H

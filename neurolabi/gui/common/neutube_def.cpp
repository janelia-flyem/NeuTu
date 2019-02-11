#include "neutube_def.h"

#include "utilities.h"

namespace neutu {

const char *VERSION = "1.1";

const char *PKG_VERSION = ""
#if defined(_PKG_VERSION)
    NT_XSTR(_PKG_VERSION)
#endif
;

std::string ToString(EBodyLabelType type)
{
  switch (type) {
  case EBodyLabelType::BODY:
    return "body";
  case EBodyLabelType::SUPERVOXEL:
    return "supervoxel";
  }

  return "";
}

}

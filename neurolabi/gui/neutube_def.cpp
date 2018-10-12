#include "neutube_def.h"
#include "core/utilities.h"

namespace neutube {

const char *VERSION = "1.1";

const char *PKG_VERSION = ""
#if defined(_PKG_VERSION)
    NT_XSTR(_PKG_VERSION)
#endif
;

}

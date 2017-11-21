#ifndef ZQSLOG_H
#define ZQSLOG_H

#if defined(VLOG)
# undef VLOG
#endif

#if defined(CHECK)
# undef CHECK
#endif

#if defined(DCHECK)
# undef DCHECK
#endif

#if defined(CHECK_EQ)
# undef CHECK_EQ
#endif

#if defined(CHECK_NE)
# undef CHECK_NE
#endif

#if defined(CHECK_LE)
# undef CHECK_LE
#endif

#if defined(CHECK_LT)
# undef CHECK_LT
#endif

#if defined(CHECK_GE)
# undef CHECK_GE
#endif

#if defined(CHECK_GT)
# undef CHECK_GT
#endif


#include "QsLog.h"

#endif // ZQSLOG_H

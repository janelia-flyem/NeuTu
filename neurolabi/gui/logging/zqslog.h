#ifndef ZQSLOG_H
#define ZQSLOG_H

#if !defined(QSLOG_H)

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

#if defined(DCHECK_EQ)
# undef DCHECK_EQ
#endif

#if defined(DCHECK_NE)
# undef DCHECK_NE
#endif

#if defined(DCHECK_LE)
# undef DCHECK_LE
#endif

#if defined(DCHECK_LT)
# undef DCHECK_LT
#endif

#if defined(DCHECK_GE)
# undef DCHECK_GE
#endif

#if defined(DCHECK_GT)
# undef DCHECK_GT
#endif

#if defined(CHECK_NOTNULL)
# undef CHECK_NOTNULL
#endif

#if defined(DCHECK_NOTNULL)
# undef DCHECK_NOTNULL
#endif

#include "QsLog.h"
#endif

#endif // ZQSLOG_H

#ifndef LIBDVIDHEADER_H
#define LIBDVIDHEADER_H

/*
#if _ENABLE_LIBDVID_
#include "DVIDNode.h"
#endif
*/

#if defined(_ENABLE_LIBDVIDCPP_)
#define nbuf /* Temporary solution for sliencing unused parameter warning */
#include "libdvid/DVIDNodeService.h"
#undef nbuf
#include "libdvid/DVIDThreadedFetch.h"
#include "libdvid/DVIDConnection.h"
#ifndef _LIBDVIDCPP_OLD_
#include "libdvid/DVIDCache.h"
#if defined(_ENABLE_LOWTIS_)
#include "lowtis/lowtis.h"
#endif
#define _LIBDVIDCPP_CACHE_
#endif
#endif

#endif // LIBDVIDHEADER_H

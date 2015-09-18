#ifndef LIBDVIDHEADER_H
#define LIBDVIDHEADER_H

/*
#if _ENABLE_LIBDVID_
#include "DVIDNode.h"
#endif
*/

#if defined(_ENABLE_LIBDVIDCPP_)
#include "libdvid/DVIDNodeService.h"
#include "libdvid/DVIDThreadedFetch.h"
#ifndef _LIBDVIDCPP_OLD_
#include "libdvid/DVIDCache.h"
#define _LIBDVIDCPP_CACHE_
#endif
#endif

#endif // LIBDVIDHEADER_H

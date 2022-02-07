#ifndef ZSKELETONIZESERVICE_H
#define ZSKELETONIZESERVICE_H

#include "tz_stdint.h"
#include "common/zsingleton.hpp"

class ZDvidTarget;

/*!
 * \brief The class for interfacing skeletonization service
 */
class ZSkeletonizeService : public ZSingleton<ZSkeletonizeService>
{
public:
  friend class ZSingleton<ZSkeletonizeService>;

  /*!
   * \brief Request skeletonizing a body
   *
   * It requests skeletonization of \a bodyId in a dvid env \a target and throws
   * a runtime error if the request fails.
   */
  void requestSkeletonize(const ZDvidTarget &target, uint64_t bodyId);
};

#endif // ZSKELETONIZESERVICE_H

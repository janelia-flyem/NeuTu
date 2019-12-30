#ifndef ZFLYEMBODYSTATEACCESSOR_H
#define ZFLYEMBODYSTATEACCESSOR_H

#include <cstdint>

/*!
 * \brief The abstract class of accessing body statuses
 */
class ZFlyEmBodyStateAccessor
{
public:
  ZFlyEmBodyStateAccessor();
  virtual ~ZFlyEmBodyStateAccessor() {}

  virtual bool isProtected(uint64_t bodyId) const = 0;

};

#endif // ZFLYEMBODYSTATEACCESSOR_H

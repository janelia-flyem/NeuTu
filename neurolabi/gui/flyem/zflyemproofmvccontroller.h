#ifndef ZFLYEMPROOFMVCCONTROLLER_H
#define ZFLYEMPROOFMVCCONTROLLER_H

#include "tz_stdint.h"

class ZFlyEmProofMvc;
class ZIntPoint;

/*!
 * \brief An experimental class for controlling mvc widgets
 */
class ZFlyEmProofMvcController
{
public:
  ZFlyEmProofMvcController();

public:
  static void GoToBody(ZFlyEmProofMvc *mvc, uint64_t bodyId);
  static void GoToPosition(ZFlyEmProofMvc *mvc, const ZIntPoint &pos);
  static void Close(ZFlyEmProofMvc *mvc);
  static void EnableHighlightMode(ZFlyEmProofMvc *mvc);
};

#endif // ZFLYEMPROOFMVCCONTROLLER_H

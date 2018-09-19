#ifndef ZFLYEMBODYENV_H
#define ZFLYEMBODYENV_H

#include "tz_stdint.h"

class Neu3Window;

/*!
 * \brief A helper class for coordinating body interactions in ZFLyEmBody3dDoc
 *
 *
 */
class ZFlyEmBodyEnv
{
public:
  ZFlyEmBodyEnv();

  void setTopWindow(Neu3Window *win);

public:
  bool allowingSplit(uint64_t bodyId) const;

private:
  Neu3Window *m_new3Win = nullptr;
};

#endif // ZFLYEMBODYENV_H

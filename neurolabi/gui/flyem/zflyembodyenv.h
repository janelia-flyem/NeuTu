#ifndef ZFLYEMBODYENV_H
#define ZFLYEMBODYENV_H

#include <cstdint>

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

  void setWindowEnv(Neu3Window *win);

public:
  bool allowingSplit(uint64_t bodyId) const;
  bool cleaving() const;

private:
  Neu3Window *m_neu3Win = nullptr;
};

#endif // ZFLYEMBODYENV_H

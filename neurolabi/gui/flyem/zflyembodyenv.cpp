#include "zflyembodyenv.h"

#include "neu3window.h"
#include "protocols/taskprotocoltask.h"

ZFlyEmBodyEnv::ZFlyEmBodyEnv()
{

}

bool ZFlyEmBodyEnv::allowingSplit(uint64_t bodyId) const
{
  if (m_neu3Win != nullptr) {
    return m_neu3Win->allowingSplit(bodyId);
  }

  return true;
}

void ZFlyEmBodyEnv::setWindowEnv(Neu3Window *win)
{
  m_neu3Win = win;
}

bool ZFlyEmBodyEnv::cleaving() const
{
  if (m_neu3Win != nullptr) {
    return m_neu3Win->cleaving();
  }

  return false;
}

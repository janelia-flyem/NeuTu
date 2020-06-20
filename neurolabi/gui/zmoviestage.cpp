#include "zmoviestage.h"
#include "z3dwindow.h"
#include "z3dvolumefilter.h"
#include "mvc/zstackdoc.h"

ZMovieStage::ZMovieStage(Z3DWindow *window) : m_window(window),
  m_isSwcChanged(false), m_isVolumeChanged(false), m_isPunctaChanged(false),
  m_isCubeArrayChanged(false)
{
}

bool ZMovieStage::hasAnyChange()
{
  return isSwcChanged() || isVolumeChanged() || isPunctaChanged() ||
      isCubeArrayChanged();
}

void ZMovieStage::updateWindow()
{
  bool changed = false;

  if (isSwcChanged()) {
    getWindow()->getDocument()->processObjectModified(ZStackObject::EType::SWC);
//    getWindow()->getDocument()->notifySwcModified();
    setSwcChanged(false);
    changed = true;
  }

  if (isVolumeChanged()) {
    getWindow()->getDocument()->notifyStackModified(true);
    setVolumeChanged(false);
    changed = true;
  }

  if (isPunctaChanged()) {
    getWindow()->getDocument()->processObjectModified(
          ZStackObject::EType::PUNCTUM);
//    getWindow()->getDocument()->notifyPunctumModified();
    setPunctaChanged(false);
    changed = true;
  }

  if (isCubeArrayChanged()) {
    getWindow()->getDocument()->processObjectModified(ZStackObject::EType::CUBE);
//    getWindow()->getDocument()->notify3DCubeModified();
    setCubeArrayChanged(false);
    changed = true;
  }

  getWindow()->resetCameraClippingRange();
}

void ZMovieStage::hideVolume()
{
  getWindow()->getVolumeFilter()->setChannel1Visible(false);
  getWindow()->getVolumeFilter()->setChannel2Visible(false);
  getWindow()->getVolumeFilter()->setChannel3Visible(false);
  getWindow()->getVolumeFilter()->setChannel4Visible(false);
}

void ZMovieStage::showVolume()
{
  getWindow()->getVolumeFilter()->setChannel1Visible(true);
  getWindow()->getVolumeFilter()->setChannel2Visible(true);
  getWindow()->getVolumeFilter()->setChannel3Visible(true);
  getWindow()->getVolumeFilter()->setChannel4Visible(true);
}

void ZMovieStage::saveScreenShot(const std::string &filePath,
                                 int width, int height)
{
  getWindow()->takeScreenShot(filePath.c_str(), width, height, Z3DScreenShotType::MonoView);
}

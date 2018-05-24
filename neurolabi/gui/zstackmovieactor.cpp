#include "zstackmovieactor.h"

#include <QApplication>
#include <iostream>
#include "zstack.hxx"
#include "zmoviestage.h"
#include "z3dwindow.h"
#include "z3dvolumefilter.h"
#include "zstackdoc.h"

using namespace std;

ZStackMovieActor::ZStackMovieActor()
{
  m_stack = NULL;
  m_type = STACK;
}

ZStackMovieActor::~ZStackMovieActor()
{
  m_stack = NULL;
}

void ZStackMovieActor::setActor(ZStack *stack)
{
  m_stack = stack;
}

void ZStackMovieActor::hide()
{
  setVisible(false);
  m_stage->hideVolume();
  //getVolumeRaycaster()->getRenderer()->setChannel1Visible(false);
}

void ZStackMovieActor::show()
{
#ifdef _DEBUG_
    if (getId() == "slice_colored") {
      cout << "debug here" << endl;
    }
#endif

  setVisible(true);
  m_stage->showVolume();
  //m_stage->getVolumeRaycaster()->getRenderer()->setChannel1Visible(true);
}

void ZStackMovieActor::move(double t)
{
  m_stage->getWindow()->getVolumeFilter()->setOffset(
        m_movingOffset[0] * t, m_movingOffset[1] * t, m_movingOffset[2] * t);
}

void ZStackMovieActor::reset()
{
  if (m_stage != NULL) {
    if (m_stack != m_stage->getWindow()->getDocument()->getStack()) {
      m_stage->getWindow()->getDocument()->loadStack(m_stack);
      QApplication::processEvents();
      //m_stage->setVolumeChanged(true);
    }
    pushColor();
    pushAlpha();
  }
}

void ZStackMovieActor::pushAlpha()
{
  if (m_stage != NULL) {
    m_stage->getWindow()->getVolumeFilter()->setOpaque(true);
    m_stage->getWindow()->getVolumeFilter()->setAlpha(m_alpha);
  }
}

void ZStackMovieActor::pushColor()
{

}

void ZStackMovieActor::pullAlpha()
{
  m_alpha = m_stage->getWindow()->getVolumeFilter()->getAlpha();
}

void ZStackMovieActor::pullColor()
{

}

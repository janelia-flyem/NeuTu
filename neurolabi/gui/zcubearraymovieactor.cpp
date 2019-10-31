#include "zcubearraymovieactor.h"

#include "common/math.h"
#include "zcubearray.h"
#include "zmoviestage.h"

ZCubeArrayMovieActor::ZCubeArrayMovieActor()
{
  m_cubeArray = NULL;
  m_type = CUBE_ARRAY;
}

ZCubeArrayMovieActor::~ZCubeArrayMovieActor()
{
  m_cubeArray = NULL;
}

void ZCubeArrayMovieActor::setActor(ZCubeArray *cubeArray)
{
  m_cubeArray = cubeArray;
}

void ZCubeArrayMovieActor::hide()
{
  if (m_cubeArray != NULL) {
    m_cubeArray->setVisible(false);
    getStage()->setCubeArrayChanged(true);
  }
}

void ZCubeArrayMovieActor::show()
{
  if (m_cubeArray != NULL) {
    m_cubeArray->setVisible(true);
    getStage()->setCubeArrayChanged(true);
  }
}

void ZCubeArrayMovieActor::move(double /*t*/)
{
  if (m_cubeArray != NULL) {
  }
}

void ZCubeArrayMovieActor::pushColor()
{
  if (m_cubeArray != NULL) {
    m_cubeArray->setColor(
          neutu::iround(m_red * 255.0),
          neutu::iround(m_green * 255.0),
          neutu::iround(m_blue * 255.0));
    m_cubeArray->pushObjectColor();
    getStage()->setCubeArrayChanged(true);
  }
}

void ZCubeArrayMovieActor::pullColor()
{
  if (m_cubeArray != NULL) {
    m_red = m_cubeArray->getRedF();
    m_green = m_cubeArray->getGreenF();
    m_blue = m_cubeArray->getBlueF();
  }
}

void ZCubeArrayMovieActor::pushAlpha()
{
  if (m_cubeArray != NULL) {
    m_cubeArray->setAlpha(neutu::iround(m_alpha * 255.0));
    getStage()->setCubeArrayChanged(true);
  }
}

void ZCubeArrayMovieActor::pullAlpha()
{
  if (m_cubeArray != NULL) {
    m_alpha = m_cubeArray->getAlphaF();
  }
}

void ZCubeArrayMovieActor::reset()
{
  pushColor();
  pushAlpha();
}

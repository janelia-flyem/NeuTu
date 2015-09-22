#include "zdvidtileupdatetaskmanager.h"
#include "dvid/zdvidtile.h"

ZDvidTileUpdateTask::ZDvidTileUpdateTask(QObject *parent, ZDvidTile *tile) :
  ZTask(parent), m_tile(tile), m_z(0)
{

}

void ZDvidTileUpdateTask::execute()
{
  if (m_tile != NULL) {
    m_tile->update(m_z);
  }
}
//////////////////////////////////////
ZDvidTileDecodeTask::ZDvidTileDecodeTask(QObject *parent, ZDvidTile *tile) :
  ZTask(parent), m_tile(tile), m_z(0)
{
  m_data = NULL;
  m_length = 0;
  m_highContrast = false;
}

void ZDvidTileDecodeTask::execute()
{
  if (m_tile != NULL && m_data != NULL) {
    m_tile->loadDvidSlice(m_data, m_length, m_z);
//#ifdef _DEBUG_
    std::cout << "1 tile loaded." << m_tile->getWidth() << "x" << m_tile->getHeight() << std::endl;
//#endif
    if (m_highContrast != m_tile->hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST)) {
      m_tile->enhanceContrast(m_highContrast);
    }
  }
}



////////////////////////////////////
ZDvidTileUpdateTaskManager::ZDvidTileUpdateTaskManager()
{
}

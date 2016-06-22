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
  ProcessDataForDisplay(m_data, m_length, m_z, m_highContrast, m_tile);
}

void ZDvidTileDecodeTask::ProcessDataForDisplay(
    const uint8_t *data, int length, int z, bool highContrast, ZDvidTile *tile)
{
  if (tile != NULL && data != NULL) {
    tile->loadDvidSlice(data, length, z, highContrast);
#ifdef _DEBUG_2
    std::cout << "1 tile loaded." << tile->getWidth() << "x" << tile->getHeight() << std::endl;
#endif
//    if (highContrast != tile->hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST)) {
//      tile->enhanceContrast(highContrast, false);
//    }
  }
}

void ZDvidTileDecodeTask::ExecuteTask(ZDvidTileDecodeTask *task)
{
  task->execute();
}


////////////////////////////////////
ZDvidTileUpdateTaskManager::ZDvidTileUpdateTaskManager()
{
}

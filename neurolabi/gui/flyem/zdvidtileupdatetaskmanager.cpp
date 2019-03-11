#include "zdvidtileupdatetaskmanager.h"
#include "dvid/zdvidtile.h"
#include "dvid/zdvidurl.h"

#if 0
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
#endif
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

//////////////////
void ZDvidGrayscaleReadTask::execute()
{
  ZDvidUrl url(m_reader.getDvidTarget());
  std::string urlStr = url.getGrayscaleUrl(m_sx, m_sy, m_x, m_y, m_z, m_format);
#ifdef _DEBUG_2
std::cout << urlStr << std::endl;
#endif
  m_reader.getBufferReader().read(urlStr.c_str(), false);
}

////////////////////////////////////512
ZDvidTileUpdateTaskManager::ZDvidTileUpdateTaskManager()
{
}

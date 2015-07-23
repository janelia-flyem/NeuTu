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
}

void ZDvidTileDecodeTask::execute()
{
  if (m_tile != NULL && m_data != NULL) {
    m_tile->loadDvidSlice(m_data, m_length, m_z);
  }
}



////////////////////////////////////
ZDvidTileUpdateTaskManager::ZDvidTileUpdateTaskManager()
{
}

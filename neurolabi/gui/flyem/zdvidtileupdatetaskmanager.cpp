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

ZDvidTileUpdateTaskManager::ZDvidTileUpdateTaskManager()
{
}

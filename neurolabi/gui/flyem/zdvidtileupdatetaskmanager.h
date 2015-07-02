#ifndef ZDVIDTILEUPDATETASKMANAGER_H
#define ZDVIDTILEUPDATETASKMANAGER_H

#include "zmultitaskmanager.h"

class ZDvidTile;

class ZDvidTileUpdateTask : public ZTask
{
public:
  ZDvidTileUpdateTask(QObject *parent, ZDvidTile *tile = NULL);
  void execute();

  void setZ(int z) { m_z = z; }

private:
  ZDvidTile *m_tile;
  int m_z;
};


class ZDvidTileUpdateTaskManager : public ZMultiTaskManager
{
public:
  ZDvidTileUpdateTaskManager();
};

#endif // ZDVIDTILEUPDATETASKMANAGER_H

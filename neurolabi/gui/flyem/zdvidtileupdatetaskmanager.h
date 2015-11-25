#ifndef ZDVIDTILEUPDATETASKMANAGER_H
#define ZDVIDTILEUPDATETASKMANAGER_H

#include "zmultitaskmanager.h"
#include "tz_stdint.h"
#include "ztask.h"

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

class ZDvidTileDecodeTask : public ZTask
{
public:
  ZDvidTileDecodeTask(QObject *parent, ZDvidTile *tile = NULL);
  void execute();

  void setZ(int z) { m_z = z; }
  void setData(const uint8_t *data, int length) {
    m_data = data;
    m_length = length;
  }
  void setHighContrast(bool state) {
    m_highContrast = state;
  }

  static void ProcessDataForDisplay(
      const uint8_t *data, int length, int z, bool highContrast, ZDvidTile *tile);
  static void ExecuteTask(ZDvidTileDecodeTask *task);

private:
  ZDvidTile *m_tile;
  const uint8_t *m_data;
  int m_length;
  int m_z;
  bool m_highContrast;
};


class ZDvidTileUpdateTaskManager : public ZMultiTaskManager
{
public:
  ZDvidTileUpdateTaskManager();
};

#endif // ZDVIDTILEUPDATETASKMANAGER_H

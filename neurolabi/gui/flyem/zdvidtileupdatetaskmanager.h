#ifndef ZDVIDTILEUPDATETASKMANAGER_H
#define ZDVIDTILEUPDATETASKMANAGER_H

#include <cstdint>

#include "zmultitaskmanager.h"
#include "ztask.h"
#include "dvid/zdvidreader.h"

class ZDvidTile;
#if 0
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
#endif

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

class ZDvidGrayscaleReadTask : public ZTask
{
public:
  ZDvidGrayscaleReadTask(QObject *parent) : ZTask(parent) {}
  void execute();

  void setDvidTarget(const ZDvidTarget &target) {
    m_reader.open(target);
  }

  void setRange(int sx, int sy, int x, int y, int z) {
    m_sx = sx;
    m_sy = sy;
    m_x = x;
    m_y = y;
    m_z = z;
  }

  void setFormat(const std::string &format) {
    m_format = format;
  }

private:
  ZDvidReader m_reader;
  int m_sx;
  int m_sy;
  int m_x;
  int m_y;
  int m_z;
  std::string m_format;
};


class ZDvidTileUpdateTaskManager : public ZMultiTaskManager
{
public:
  ZDvidTileUpdateTaskManager();
};

#endif // ZDVIDTILEUPDATETASKMANAGER_H

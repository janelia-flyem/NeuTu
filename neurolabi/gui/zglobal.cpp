#include "zglobal.h"

#include <map>

#include "zintpoint.h"
#include "zpoint.h"
#if defined(_QT_GUI_USED_)
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#endif

#include "neutubeconfig.h"

class ZGlobalData {

public:
  ZGlobalData();

  ZIntPoint m_stackPosition;
  std::map<std::string, ZDvidReader*> m_dvidReaderMap;
  std::map<std::string, ZDvidWriter*> m_dvidWriterMap;
};

ZGlobalData::ZGlobalData()
{
  m_stackPosition.invalidate();
}


ZGlobal::ZGlobal()
{
  m_data = new ZGlobalData;
}

ZGlobal::~ZGlobal()
{
  delete m_data;
  m_data = NULL;
}

void ZGlobal::setStackPosition(int x, int y, int z)
{
  m_data->m_stackPosition.set(x, y, z);
}

ZIntPoint ZGlobal::getStackPosition() const
{
  return m_data->m_stackPosition;
}

void ZGlobal::setStackPosition(const ZIntPoint &pt)
{
  m_data->m_stackPosition = pt;
}

void ZGlobal::setStackPosition(const ZPoint &pt)
{
  setStackPosition(pt.toIntPoint());
}

void ZGlobal::clearStackPosition()
{
  m_data->m_stackPosition.invalidate();
}

template<typename T>
T* ZGlobal::getIODevice(
    const std::string &name, std::map<std::string, T*> &ioMap) const
{
  T *io = NULL;

#if defined(_QT_GUI_USED_)
  if (ioMap.count(name) == 0) {
    const std::vector<ZDvidTarget> &dvidRepo = GET_FLYEM_CONFIG.getDvidRepo();
    for (std::vector<ZDvidTarget>::const_iterator iter = dvidRepo.begin();
         iter != dvidRepo.end(); ++iter) {
      const ZDvidTarget &target = *iter;
      if (target.getName() == name) {
        if (target.isValid()) {
          io = new T;
          if (!io->open(target)) {
            delete io;
            io = NULL;
          }
        }
      }
    }

    ioMap[name] = io;
  } else {
    io = ioMap[name];
  }
#endif

  return io;
}

ZDvidReader* ZGlobal::getDvidReader(const std::string &name) const
{
  return getIODevice(name, m_data->m_dvidReaderMap);
}

ZDvidWriter* ZGlobal::getDvidWriter(const std::string &name) const
{
  return getIODevice(name, m_data->m_dvidWriterMap);
}

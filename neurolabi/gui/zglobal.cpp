#include "zglobal.h"

#include <QUrl>
#include <map>

#include "zintpoint.h"
#include "zpoint.h"
#include "zstring.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "zdvidutil.h"
#include "sandbox/zbrowseropener.h"

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
  m_browserOpener = ZSharedPointer<ZBrowserOpener>(new ZBrowserOpener);
  m_browserOpener->setBrowserPath(ZBrowserOpener::INTERNAL_BROWSER);
}

ZGlobal::~ZGlobal()
{
  delete m_data;
  m_data = NULL;
}


ZBrowserOpener* ZGlobal::getBrowserOpener() const
{
  return m_browserOpener.get();
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

    if (io == NULL) {
      ZDvidTarget target;
      target.setFromSourceString(name);
      if (target.isValid()) {
        io = new T;
        if (!io->open(target)) {
          delete io;
          io = NULL;
        }
      }
    }

    ioMap[name] = io;
  } else {
    io = ioMap[name];
  }

  return io;
}

template<typename T>
T* ZGlobal::getIODevice(const ZDvidTarget &target,
                        std::map<std::string, T*> &ioMap,
                        const std::string &key) const
{
  T *device = NULL;
  if (target.isValid()) {
    device = getIODevice(target.getSourceString(true) + "$" + key, ioMap);
  }

  return device;
}

template<typename T>
T* ZGlobal::getIODeviceFromUrl(
    const std::string &path, std::map<std::string, T*> &ioMap,
    const std::string &key) const
{
  T *device = NULL;

  QUrl url(path.c_str());
  if (url.scheme() == "http" || url.scheme() == "dvid") {
    ZDvidTarget target = ZDvid::MakeTargetFromUrl(path);
    return getIODevice(target, ioMap, key);
//    device = getIODevice(target.getSourceString(true), ioMap);
  }

  return device;
}

ZDvidReader* ZGlobal::getDvidReader(const std::string &name) const
{
  return getIODevice(name, m_data->m_dvidReaderMap);
}

ZDvidWriter* ZGlobal::getDvidWriter(const std::string &name) const
{
  return getIODevice(name, m_data->m_dvidWriterMap);
}

ZDvidReader* ZGlobal::getDvidReader(
    const ZDvidTarget &target, const std::string &key) const
{
  return getIODevice(target, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriter(
    const ZDvidTarget &target, const std::string &key) const
{
  return getIODevice(target, m_data->m_dvidWriterMap, key);
}

ZDvidReader* ZGlobal::getDvidReaderFromUrl(
    const std::string &url, const std::string &key) const
{
  return getIODeviceFromUrl(url, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriterFromUrl(
    const std::string &url, const std::string &key) const
{
  return getIODeviceFromUrl(url, m_data->m_dvidWriterMap, key);
}

ZDvidSparseStack* ZGlobal::readDvidSparseStack(const std::string &url) const
{
  ZDvidSparseStack *spStack = NULL;

  uint64_t bodyId = ZDvidUrl::GetBodyId(url);

  if (bodyId > 0) {
    ZDvidReader *reader = getDvidReaderFromUrl(url);
    if (reader != NULL) {
      if (reader->getDvidTarget().hasBodyLabel()) {
        spStack = reader->readDvidSparseStack(bodyId);
      }
    }
  }

  return spStack;
}

ZDvidReader* ZGlobal::GetDvidReader(const std::string &name)
{
  return GetInstance().getDvidReader(name);
}

ZDvidWriter* ZGlobal::GetDvidWriter(const std::string &name)
{
  return GetInstance().getDvidWriter(name);
}

ZDvidReader* ZGlobal::GetDvidReader(
    const ZDvidTarget &target, const std::string &key)
{
  return GetInstance().getDvidReader(target, key);
}

ZDvidWriter* ZGlobal::GetDvidWriter(
    const ZDvidTarget &target, const std::string &key)
{
  return GetInstance().getDvidWriter(target, key);
}

ZDvidReader* ZGlobal::GetDvidReaderFromUrl(
    const std::string &url, const std::string &key)
{
  return GetInstance().getDvidReaderFromUrl(url, key);
}

ZDvidWriter* ZGlobal::GetDvidWriterFromUrl(
    const std::string &url, const std::string &key)
{
  return GetInstance().getDvidWriterFromUrl(url, key);
}

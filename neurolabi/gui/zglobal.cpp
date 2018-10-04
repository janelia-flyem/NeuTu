#include "zglobal.h"

#include <QUrl>
#include <map>
#include <QClipboard>
#include <QApplication>

#include "zintpoint.h"
#include "zpoint.h"
#include "zstring.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "zdvidutil.h"
#include "sandbox/zbrowseropener.h"
#include "flyem/zglobaldvidrepo.h"

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
  m_browserOpener->setChromeBrowser();
//  m_browserOpener->setBrowserPath(ZBrowserOpener::INTERNAL_BROWSER);
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

void ZGlobal::setDataPosition(int x, int y, int z)
{
  m_data->m_stackPosition.set(x, y, z);
}

ZIntPoint ZGlobal::getStackPosition() const
{
  return m_data->m_stackPosition;
}

void ZGlobal::setDataPosition(const ZIntPoint &pt)
{
  m_data->m_stackPosition = pt;
}

void ZGlobal::setDataPosition(const ZPoint &pt)
{
  setDataPosition(pt.toIntPoint());
}

void ZGlobal::clearStackPosition()
{
  m_data->m_stackPosition.invalidate();
}

void ZGlobal::setMainWindow(QMainWindow *win)
{
  m_mainWin = win;
}

QMainWindow* ZGlobal::getMainWindow() const
{
  return m_mainWin;
}

template<typename T>
T* ZGlobal::getIODevice(
    const std::string &name, std::map<std::string, T*> &ioMap,
    const std::string &key) const
{
  T *io = NULL;

  std::string nameKey = name + "$" + key;
  if (ioMap.count(nameKey) == 0) {
    {
      const ZDvidTarget &target =
          ZGlobalDvidRepo::GetInstance().getDvidTarget(name);
      if (target.isValid()) {
        io = new T;
        if (!io->open(target)) {
          delete io;
          io = NULL;
        }
      }
    }
/*
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
*/
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

    ioMap[nameKey] = io;
  } else {
    io = ioMap[nameKey];
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
    device = getIODevice(target.getSourceString(true), ioMap, key);
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
  if (url.scheme() == "http" || url.scheme() == "dvid" ||
      url.scheme() == "mock") {
    ZDvidTarget target = ZDvid::MakeTargetFromUrl(path);
    return getIODevice(target, ioMap, key);
//    device = getIODevice(target.getSourceString(true), ioMap);
  }

  return device;
}

ZDvidReader* ZGlobal::getDvidReader(
    const std::string &name, const std::string &key) const
{
  return getIODevice(name, m_data->m_dvidReaderMap, key);
}

ZDvidWriter* ZGlobal::getDvidWriter(
    const std::string &name, const std::string &key) const
{
  return getIODevice(name, m_data->m_dvidWriterMap, key);
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
        spStack = reader->readDvidSparseStack(
              bodyId, flyem::EBodyLabelType::BODY);
      }
    }
  }

  return spStack;
}

ZDvidReader* ZGlobal::GetDvidReader(
    const std::string &name, const std::string &key)
{
  return GetInstance().getDvidReader(name, key);
}

ZDvidWriter* ZGlobal::GetDvidWriter(
    const std::string &name, const std::string &key)
{
  return GetInstance().getDvidWriter(name, key);
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

void ZGlobal::CopyToClipboard(const std::string &str)
{
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(str.c_str());
}


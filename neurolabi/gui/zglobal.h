#ifndef ZGLOBAL_H
#define ZGLOBAL_H

#include <string>
#include <map>

#include "zsharedpointer.h"

class ZIntPoint;
class ZPoint;
class ZGlobalData;
class ZDvidReader;
class ZDvidWriter;
class ZDvidSparseStack;
class ZDvidTarget;
class ZBrowserOpener;

class ZGlobal
{
public:
  ZGlobal();
  ~ZGlobal();

  static ZGlobal& GetInstance() {
    static ZGlobal g;

    return g;
  }

  void setStackPosition(int x, int y, int z);
  void setStackPosition(const ZIntPoint &pt);
  void setStackPosition(const ZPoint &pt);
  void clearStackPosition();
  ZIntPoint getStackPosition() const;
  ZDvidReader* getDvidReader(const std::string &name) const;
  ZDvidWriter* getDvidWriter(const std::string &name) const;
  ZDvidReader* getDvidReader(const ZDvidTarget &target) const;
  ZDvidWriter* getDvidWriter(const ZDvidTarget &target) const;
  ZDvidReader* getDvidReaderFromUrl(const std::string &url) const;
  ZDvidWriter* getDvidWriterFromUrl(const std::string &url) const;

public:
  static ZDvidReader* GetDvidReader(const std::string &name);
  static ZDvidWriter* GetDvidWriter(const std::string &name);
  static ZDvidReader* GetDvidReader(const ZDvidTarget &target);
  static ZDvidWriter* GetDvidWriter(const ZDvidTarget &target);
  static ZDvidReader* GetDvidReaderFromUrl(const std::string &url);
  static ZDvidWriter* GetDvidWriterFromUrl(const std::string &url);

public:
  ZDvidSparseStack* readDvidSparseStack(const std::string &url) const;
  ZBrowserOpener* getBrowserOpener() const;

private:
  template<typename T>
  T* getIODevice(
      const std::string &name, std::map<std::string, T*> &ioMap) const;

  template<typename T>
  T* getIODevice(
      const ZDvidTarget &name, std::map<std::string, T*> &ioMap) const;

  template<typename T>
  T* getIODeviceFromUrl(
      const std::string &path, std::map<std::string, T*> &ioMap) const;

private:
  ZGlobalData *m_data;
  ZSharedPointer<ZBrowserOpener> m_browserOpener;
};

#endif // ZGLOBAL_H

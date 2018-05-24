#ifndef ZGLOBAL_H
#define ZGLOBAL_H

#include <string>
#include <map>
#include <QObject>

#include "zsharedpointer.h"

class ZIntPoint;
class ZPoint;
class ZGlobalData;
class ZDvidReader;
class ZDvidWriter;
class ZDvidSparseStack;
class ZDvidTarget;
class ZBrowserOpener;
class QMainWindow;

class ZGlobal
{
public:
  ZGlobal();
  ~ZGlobal();

  static ZGlobal& GetInstance() {
    static ZGlobal g;

    return g;
  }

  /*!
   * \brief For transferring positions globally.
   */
  void setDataPosition(int x, int y, int z);
  void setDataPosition(const ZIntPoint &pt);
  void setDataPosition(const ZPoint &pt);

  void clearStackPosition();
  ZIntPoint getStackPosition() const;

  //! DVID IO with shared readers and writers
  /*!
   * \brief Get DVID reader from a name
   * \param name The name can be an alias name or dvid source string
   * \param key Additional key
   */
  ZDvidReader* getDvidReader(
      const std::string &name, const std::string &key = "") const;
  ZDvidWriter* getDvidWriter(
      const std::string &name, const std::string &key = "") const;
  ZDvidReader* getDvidReader(
      const ZDvidTarget &target, const std::string &key = "") const;
  ZDvidWriter* getDvidWriter(
      const ZDvidTarget &target, const std::string &key = "") const;
  ZDvidReader* getDvidReaderFromUrl(
      const std::string &url, const std::string &key = "") const;
  ZDvidWriter* getDvidWriterFromUrl(
      const std::string &url, const std::string &key = "") const;

  void setMainWindow(QMainWindow *win);
  template<typename T>
  T* getMainWindow() const;

  QMainWindow* getMainWindow() const;

public:
  static ZDvidReader* GetDvidReader(
      const std::string &name, const std::string &key = "");
  static ZDvidWriter* GetDvidWriter(
      const std::string &name, const std::string &key = "");
  static ZDvidReader* GetDvidReader(
      const ZDvidTarget &target, const std::string &key = "");
  static ZDvidWriter* GetDvidWriter(
      const ZDvidTarget &target, const std::string &key = "");
  static ZDvidReader* GetDvidReaderFromUrl(
      const std::string &url, const std::string &key = "");
  static ZDvidWriter* GetDvidWriterFromUrl(
      const std::string &url, const std::string &key = "");

public:
  ZDvidSparseStack* readDvidSparseStack(const std::string &url) const;
  ZBrowserOpener* getBrowserOpener() const;

  static void CopyToClipboard(const std::string &str);

private:
  /*
  template<typename T>
  T* getIODevice(
      const std::string &name, std::map<std::string, T*> &ioMap) const;
*/
  template<typename T>
  T* getIODevice(
      const std::string &name, std::map<std::string, T*> &ioMap,
      const std::string &key = "") const;

  template<typename T>
  T* getIODevice(
      const ZDvidTarget &target, std::map<std::string, T*> &ioMap,
      const std::string &key) const;

  template<typename T>
  T* getIODeviceFromUrl(
      const std::string &path, std::map<std::string, T*> &ioMap,
      const std::string &key) const;

private:
  ZGlobalData *m_data;
  ZSharedPointer<ZBrowserOpener> m_browserOpener;
  QMainWindow *m_mainWin = nullptr;
};

template<typename T>
T* ZGlobal::getMainWindow() const
{
  if (m_mainWin == nullptr) {
    return nullptr;
  }

  return qobject_cast<T*>(m_mainWin);
}

#endif // ZGLOBAL_H

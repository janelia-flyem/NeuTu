#ifndef ZROIPROVIDER_H
#define ZROIPROVIDER_H

#include <vector>
#include <string>
#include <memory>

#include <QObject>
#include <QColor>
#include <QMutex>

#include "zcolorscheme.h"
#include "concurrent/zworkerwrapper.h"

class ZAbstractRoiFactory;
class ZRoiMesh;
class ZWorker;
class ZWorkThread;
class ZTask;
class ZMesh;

class ZRoiProvider : public QObject, public ZWorkerWrapper
{
  Q_OBJECT

public:
  ZRoiProvider(QObject *parent = nullptr);
  ~ZRoiProvider() override;

//  void startWorkThread();
//  void endWorkThread();

  void setRoiFactory(ZAbstractRoiFactory *factory);
  void setRoiList(const std::vector<std::string> &roiNameList);

  size_t getRoiCount() const;
  std::string getRoiName(size_t index) const;
  std::string getRoiStatus(size_t index) const;
  bool isVisible(size_t index) const;
  QColor getDefaultRoiColor(size_t index) const;
  QColor getRoiColor(size_t index) const;

  std::shared_ptr<ZRoiMesh> getSharedRoiMesh(size_t index) const;
  std::shared_ptr<ZRoiMesh> getSharedRoiMesh(const std::string &name) const;

  ZRoiMesh* getRoiMesh(size_t index) const;
  ZRoiMesh* getRoiMesh(const std::string &name) const;

  bool isRoiMeshReady(size_t index) const;

  ZMesh* makeRoiMesh(const std::string &name);

  void setVisible(size_t index, bool on);
  void setColor(size_t index, const QColor &color);

signals:
//  void roiUpdated(const QString &name);
  void roiLoaded(const QString &name);
  void roiUpdated();
  void roiListUpdated();

public slots:
  void requestRoi(int index);
//  void toggleRoiRequest(int index, bool checked);

public:
  static const char* ROI_STATUS_READY;
  static const char* ROI_STATUS_EMPTY;
  static const char* ROI_STATUS_PENDING;

private:
//  void addTask(ZTask *task);
  void requestRoi(const std::string &name);
  void requestRoi(const std::shared_ptr<ZRoiMesh> &roiMesh);
  void emitRoiLoaded(const QString &name);

private:
  ZAbstractRoiFactory *m_roiFactory = nullptr;
  std::vector<std::shared_ptr<ZRoiMesh>> m_roiList;
  ZColorScheme m_colorScheme;

  QMutex m_roiListMutex;

//  ZWorker *m_worker = nullptr;
//  ZWorkThread *m_workThread = nullptr;
};

#endif // ZROIPROVIDER_H

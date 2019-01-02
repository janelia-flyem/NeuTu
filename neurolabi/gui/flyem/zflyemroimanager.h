#ifndef ZFLYEMROIMANAGER_H
#define ZFLYEMROIMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>

#include "dvid/zdvidwriter.h"
//#include "zthreadfuturemap.h"

class ZMesh;
//class ZWorker;
//class ZWorkThread;
class ZTask;

class ZFlyEmRoiManager : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmRoiManager(QObject *parent = nullptr);
  ~ZFlyEmRoiManager();

  enum class ESource {
    ROI_DATA, REF_DATA
  };

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidReader& getDvidReader() const;
  ZDvidWriter& getDvidWriter();

  QStringList getRoiList() const;
  void loadRoiList();
  bool hasRoi(const QString &name) const;

  ZMesh* getMesh(const QString &str) const;
  void setMesh(const QString &str, ZMesh *mesh);
  void updateMesh(const QString &name);

  ZTask* makeRoiUpdateTask(const QString &name);

  void print() const;

signals:
  void roiMeshUpdated(const QString &roiName);

public slots:

private:
//  void startWorkThread();
//  void endWorkThread();
//  void addTask(ZTask *task);
//  void addTaskSlot(ZTask *task);

//  void updateMeshFromRoi(const QStringList &keyList);
//  void updateMeshFromMesh(const QStringList &keyList);

  void updateMeshFromRefData(const QString &roiName);
  void updateMeshFromRoiData(const QString &roiName);
  void loadRoi(const QString &roiName,
               const QString &key, const QString &source);
  void loadRoi(const QString &roiName,
               const QStringList &keyList, const QString &source);


private:
  ZDvidWriter m_dvidWriter;
  QStringList m_roiList;
  ESource m_source;
  QMap<QString, ZMesh*> m_meshMap;
  mutable QMutex m_meshMapMutex;
//  ZThreadFutureMap m_futureMap;
//  ZWorker *m_worker = nullptr;
//  ZWorkThread *m_workThread = nullptr;
};

#endif // ZFLYEMROIMANAGER_H

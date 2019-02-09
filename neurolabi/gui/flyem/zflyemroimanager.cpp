#include "zflyemroimanager.h"

#include <QMutexLocker>

#include <iostream>

#include "zmesh.h"
//#include "concurrent/zworker.h"
//#include "concurrent/zworkthread.h"
#include "ztask.h"
#include "zobject3dscan.h"
#include "zmeshfactory.h"
#include "zjsonobject.h"
#include "zjsondef.h"

namespace {

class UploadMeshTask : public ZTask
{
//  Q_OBJECT
public:
  explicit UploadMeshTask(QObject *parent = nullptr) : ZTask(parent) {}

  void setRoiManager(ZFlyEmRoiManager *m) {
    m_roiManager = m;
  }

  void setRoiName(const QString &name) {
    m_roiName = name;
  }

  void execute() {
    if (m_roiManager) {
      m_roiManager->updateMesh(m_roiName);
    }
  }

private:
  ZFlyEmRoiManager *m_roiManager = nullptr;
  QString m_roiName;

};

}

ZFlyEmRoiManager::ZFlyEmRoiManager(QObject *parent) : QObject(parent)
{
}

ZFlyEmRoiManager::~ZFlyEmRoiManager()
{
//  endWorkThread();
  for (ZMesh *mesh : m_meshMap) {
    delete mesh;
  }
}

ZTask* ZFlyEmRoiManager::makeRoiUpdateTask(const QString &name)
{
  UploadMeshTask *task = nullptr;
  if (hasRoi(name)) {
    task = new UploadMeshTask;
    task->setRoiManager(this);
    task->setRoiName(name);
  }

  return task;
}

/*
void ZFlyEmRoiManager::endWorkThread()
{
  if (m_worker != NULL) {
    m_worker->quit();
    m_worker = NULL;
  }
  if (m_workThread != NULL) {
    m_workThread->quit();
    m_workThread->wait();
    m_workThread = NULL;
  }
}

void ZFlyEmRoiManager::startWorkThread()
{
  if (m_workThread == NULL) {
    m_worker = new ZWorker(ZWorker::EMode::SCHEDULE);
    m_workThread = new ZWorkThread(m_worker);
    connect(m_workThread, SIGNAL(finished()), m_workThread, SLOT(deleteLater()));
    m_workThread->start();
  }
}

void ZFlyEmRoiManager::addTask(ZTask *task)
{
//  LDEBUG() << "Task added in thread: " << QThread::currentThreadId();
  if (m_worker != NULL) {
    if (task->getDelay() > 0) {
      if (m_worker->getMode() == ZWorker::EMode::QUEUE) {
        QTimer::singleShot(task->getDelay(), this, [=]() {
          this->addTaskSlot(task);
        });
      } else {
        addTaskSlot(task);
      }
    } else {
      addTaskSlot(task);
    }
  }
}

void ZFlyEmRoiManager::addTaskSlot(ZTask *task)
{
//  task->moveToThread(m_worker->thread());
  if (m_worker != NULL) {
    m_worker->addTask(task);
  }
}
*/

void ZFlyEmRoiManager::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidWriter.open(target);
//  startWorkThread();
}

const ZDvidReader &ZFlyEmRoiManager::getDvidReader() const
{
  return m_dvidWriter.getDvidReader();
}

ZDvidWriter& ZFlyEmRoiManager::getDvidWriter()
{
  return m_dvidWriter;
}

QStringList ZFlyEmRoiManager::getRoiList() const
{
  return m_roiList;
}

bool ZFlyEmRoiManager::hasRoi(const QString &name) const
{
  return m_meshMap.contains(name);
}

void ZFlyEmRoiManager::loadRoiList()
{
  m_roiList = getDvidReader().readKeys(
        ZDvidData::GetName(ZDvidData::ERole::ROI_KEY).c_str());
  m_source = ESource::REF_DATA;

  if (m_roiList.isEmpty()) {
    ZJsonObject meta = getDvidReader().readInfo();

    //
    ZJsonValue datains = meta.value("DataInstances");

    if(datains.isObject()) {
      ZJsonObject insList(datains);
      std::vector<std::string> keys = insList.getAllKey();

      for(std::size_t i=0; i<keys.size(); i++) {

        std::string roiName = keys.at(i);
        ZJsonObject roiJson(insList.value(roiName.c_str()));
        if (roiJson.hasKey("Base")) {
          ZJsonObject baseJson(roiJson.value("Base"));
          std::string typeName =
              ZJsonParser::stringValue(baseJson["TypeName"]);
          if (typeName == "roi") {
            m_roiList.append(roiName.c_str());
          }
        }
      }
    }
    m_source = ESource::ROI_DATA;
  }

  for (const QString &name : m_roiList) {
    m_meshMap[name] = nullptr;
  }
}

ZMesh* ZFlyEmRoiManager::getMesh(const QString &str) const
{
  QMutexLocker lock(&m_meshMapMutex);

  return m_meshMap.value(str, nullptr);
}

void ZFlyEmRoiManager::setMesh(const QString &str, ZMesh *mesh)
{
  QMutexLocker lock(&m_meshMapMutex);

  if (m_meshMap.value(str, nullptr) != mesh) {
    delete m_meshMap.value(str, nullptr);
    m_meshMap[str] = mesh;
    emit roiMeshUpdated(str);
  }
}

void ZFlyEmRoiManager::updateMeshFromRefData(const QString &roiName)
{
  ZJsonObject roiInfo = getDvidReader().readJsonObjectFromKey(
        ZDvidData::GetName(ZDvidData::ERole::ROI_KEY).c_str(), roiName);
  if (roiInfo.hasKey(neutu::json::REF_KEY)) {
    ZJsonObject jsonObj(roiInfo.value(neutu::json::REF_KEY));

    std::string type = ZJsonParser::stringValue(jsonObj["type"]);
    if (type.empty()) {
      type = "mesh";
    }

    if (ZJsonParser::IsArray(jsonObj["key"])) {
      ZJsonArray arrayJson(jsonObj.value("key"));
      QStringList keyList;
      for (size_t i = 0; i < arrayJson.size(); ++i) {
        std::string key = ZJsonParser::stringValue(arrayJson.at(i));
        if (!key.empty()) {
          keyList.push_back(key.c_str());
        }
      }
      loadRoi(roiName, keyList, type.c_str());

    } else {
      std::string key = ZJsonParser::stringValue(jsonObj["key"]);
      if (key.empty()) {
        key = roiName.toStdString();
      }
      loadRoi(roiName, key.c_str(), type.c_str());
    }
  }
}

void ZFlyEmRoiManager::updateMeshFromRoiData(const QString &roiName)
{
  loadRoi(roiName, roiName, "roi");
}

void ZFlyEmRoiManager::loadRoi(
    const QString &roiName, const QString &key, const QString &source)
{
  if (!roiName.isEmpty() && !key.isEmpty()) {
    ZMesh *mesh = NULL;

#ifdef _DEBUG_
    qDebug() << "Add ROI: " << "from " << key << " (" << source << ")"
              << " as " << roiName;
#endif

    if (source == "roi") {
      ZObject3dScan roi;
      getDvidReader().readRoi(key.toStdString(), &roi);
      if (!roi.isEmpty()) {
        ZMeshFactory mf;
        mf.setOffsetAdjust(true);
        mesh = mf.makeMesh(roi);
      }
    } else if (source == "mesh") {
      mesh = getDvidReader().readMesh(
            ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY),
            key.toStdString());
    }
    setMesh(roiName, mesh);
  }
}

void ZFlyEmRoiManager::loadRoi(const QString &roiName, const QStringList &keyList,
    const QString &source)
{
  if (!roiName.isEmpty() && !keyList.isEmpty()) {
    ZMesh *mesh = NULL;

#ifdef _DEBUG_
    qDebug() << "Add ROI: " << "from " << " (" << source << ")"
              << " as " << roiName;
#endif

    if (source == "roi") {
      ZObject3dScan roi;
      for (const QString &key : keyList) {
        getDvidReader().readRoi(key.toStdString(), &roi, true);
      }
      if (!roi.isEmpty()) {
        ZMeshFactory mf;
        mf.setOffsetAdjust(true);
        mesh = mf.makeMesh(roi);
      }
    } else if (source == "mesh") {
      if (keyList.size() == 1) {
        mesh = getDvidReader().readMesh(
              ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY),
              keyList[0].toStdString());
      } else {
        std::vector<ZMesh*> meshList;
        for (const QString &key : keyList) {
          ZMesh *submesh = getDvidReader().readMesh(
                ZDvidData::GetName(ZDvidData::ERole::ROI_DATA_KEY),
                key.toStdString());
          if (submesh != NULL) {
            meshList.push_back(submesh);
          }
        }
        if (!meshList.empty()) {
          mesh = new ZMesh;
          *mesh = ZMesh::Merge(meshList);
          for (ZMesh *submesh : meshList) {
            delete submesh;
          }
        }
      }
    }

    setMesh(roiName, mesh);
  }
}

/*
void ZFlyEmRoiManager::updateMeshFromRoi(const QStringList &keyList)
{
  ZObject3dScan roi;
  for (const QString &name : keyList) {
    getDvidReader().readRoi(name.toStdString(), &roi, true);
  }
  if (!roi.isEmpty()) {
    setMesh(name, mesh);
  }
}

void ZFlyEmRoiManager::updateMeshFromMesh(const QStringList &keyList)
{
  ZMesh *mesh = nullptr;

  if (keyList.size() == 1) {
    mesh = reader.readMesh(
          ZDvidData::GetName(ZDvidData::ROLE_ROI_DATA_KEY), keyList[0]);
  } else {
    std::vector<ZMesh*> meshList;
    for (const std::string &key : keyList) {
      ZMesh *submesh = reader.readMesh(
            ZDvidData::GetName(ZDvidData::ROLE_ROI_DATA_KEY), key);
      if (submesh != NULL) {
        meshList.push_back(submesh);
      }
    }
    if (!meshList.empty()) {
      mesh = new ZMesh;
      *mesh = ZMesh::Merge(meshList);
      for (ZMesh *submesh : meshList) {
        delete submesh;
      }
    }
  }

  setMesh(name, mesh);
}
*/

void ZFlyEmRoiManager::updateMesh(const QString &name)
{
  if (m_meshMap.contains(name)) {
    switch (m_source) {
    case ESource::REF_DATA:
      updateMeshFromRefData(name);
      break;
    case ESource::ROI_DATA:
      updateMeshFromRoiData(name);
      break;
    }
  }
}

void ZFlyEmRoiManager::print() const
{
  for (auto iter = m_meshMap.begin(); iter != m_meshMap.end(); ++iter) {
    std::cout << iter.key().toStdString() << ": " << iter.value() << std::endl;
  }
}

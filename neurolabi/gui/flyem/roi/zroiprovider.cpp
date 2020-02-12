#include "zroiprovider.h"

#include <QMutexLocker>

#include "zabstractroifactory.h"

#include "concurrent/zworker.h"
#include "concurrent/zworkthread.h"
#include "ztask.h"
#include "zroimesh.h"
#include "zmesh.h"

const char* ZRoiProvider::ROI_STATUS_READY = "Ready";
const char* ZRoiProvider::ROI_STATUS_EMPTY = "Empty";
const char* ZRoiProvider::ROI_STATUS_PENDING = "Pending";

ZRoiProvider::ZRoiProvider(QObject *parent) : QObject(parent)
{
  m_colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
  connect(this, SIGNAL(roiUpdated(const QString&)), this, SIGNAL(roiUpdated()));
}

ZRoiProvider::~ZRoiProvider()
{
  delete m_roiFactory;
  endWorkThread();
}

void ZRoiProvider::endWorkThread()
{
  if (m_workThread) {
    m_workThread->cancelAndQuit();
  }
  m_worker = nullptr;
  m_workThread = nullptr;
}

void ZRoiProvider::startWorkThread()
{
  if (m_workThread == nullptr) {
    m_worker = new ZWorker(ZWorker::EMode::SCHEDULE);
    m_workThread = new ZWorkThread(m_worker);
    connect(m_workThread, SIGNAL(finished()), m_workThread, SLOT(deleteLater()));
    m_workThread->start();
  }
}

void ZRoiProvider::setRoiFactory(ZAbstractRoiFactory *factory)
{
  endWorkThread();
  delete m_roiFactory;
  m_roiFactory = factory;
}

void ZRoiProvider::setRoiList(const std::vector<std::string> &roiNameList)
{
  endWorkThread();
  QMutexLocker locker(&m_roiListMutex);
  m_roiList.clear();
  m_roiList.resize(roiNameList.size());
  for (size_t i = 0; i < roiNameList.size(); ++i) {
    m_roiList[i] = std::shared_ptr<ZRoiMesh>(
          new ZRoiMesh(roiNameList[i], getDefaultRoiColor(i)));
  }

  emit roiListUpdated();
}


size_t ZRoiProvider::getRoiCount() const
{
  return m_roiList.size();
}

std::string ZRoiProvider::getRoiName(size_t index) const
{
  if (index < m_roiList.size()) {
    return m_roiList[index]->getName();
  }

  return "";
}

bool ZRoiProvider::isRoiMeshReady(size_t index) const
{
  ZRoiMesh *roiMesh = getRoiMesh(index);
  if (roiMesh) {
    return (roiMesh->getStatus() == ZRoiMesh::EStatus::READY);
  }

  return false;
}

std::string ZRoiProvider::getRoiStatus(size_t index) const
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    switch (mesh->getStatus()) {
    case ZRoiMesh::EStatus::READY:
      return ROI_STATUS_READY;
    case ZRoiMesh::EStatus::EMPTY:
      return ROI_STATUS_EMPTY;
    case ZRoiMesh::EStatus::PENDING:
      return ROI_STATUS_PENDING;
    }
  }

  return "N/A";
}

bool ZRoiProvider::isVisible(size_t index) const
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    return mesh->isVisible();
  }

  return false;
}

/*
void ZRoiProvider::toggleRoiRequest(int index, bool checked)
{
  if (index >= 0) {
    if (checked) {
      requestRoi(size_t(index));
    } else {
      //Todo: delay roi request
    }
  }
}
*/

QColor ZRoiProvider::getDefaultRoiColor(size_t index) const
{
  return m_colorScheme.getColor(int(index));
}

QColor ZRoiProvider::getRoiColor(size_t index) const
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    return mesh->getColor();
  }

  return getDefaultRoiColor(index);
}

void ZRoiProvider::setColor(size_t index, const QColor &color)
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    mesh->setColor(color);
  }
}

void ZRoiProvider::setVisible(size_t index, bool on)
{
  ZRoiMesh *mesh = getRoiMesh(index);
  if (mesh) {
    mesh->setVisible(on);
  }
}

std::shared_ptr<ZRoiMesh> ZRoiProvider::getSharedRoiMesh(size_t index) const
{
  if (index < m_roiList.size()) {
    return m_roiList[index];
  }

  return std::shared_ptr<ZRoiMesh>();
}

std::shared_ptr<ZRoiMesh> ZRoiProvider::getSharedRoiMesh(const std::string &name) const
{
  if (!name.empty()) {
    for (const auto &roiMesh : m_roiList) {
      if (roiMesh->getName() == name) {
        return roiMesh;
      }
    }
  }

  return std::shared_ptr<ZRoiMesh>();
}

ZRoiMesh* ZRoiProvider::getRoiMesh(size_t index) const
{
  if (index < m_roiList.size()) {
    return m_roiList[index].get();
  }

  return nullptr;
}

ZRoiMesh* ZRoiProvider::getRoiMesh(const std::string &name) const
{
  if (!name.empty()) {
    for (const auto &roiMesh : m_roiList) {
      if (roiMesh->getName() == name) {
        return roiMesh.get();
      }
    }
  }

  return nullptr;
}

void ZRoiProvider::emitRoiLoaded(const QString &name)
{
  emit roiLoaded(name);
}

void ZRoiProvider::addTask(ZTask *task)
{
  if (m_workThread) {
    m_workThread->addTask(task);
  } else {
    delete task;
  }
}

void ZRoiProvider::requestRoi(const std::shared_ptr<ZRoiMesh> &roiMesh)
{
  if (roiMesh) {
    if (roiMesh->getStatus() == ZRoiMesh::EStatus::PENDING) {
      ZFunctionTask *task = new ZFunctionTask([roiMesh, this]() {
        if (m_roiFactory) {
//          QMutexLocker locker(&m_roiListMutex);
          ZMesh *mesh = m_roiFactory->makeRoiMesh(roiMesh->getName());
          if (mesh == nullptr) {
            mesh = new ZMesh();
          }
          roiMesh->setMesh(mesh);
          this->emitRoiLoaded(roiMesh->getName().c_str());
        }
      });
      task->skipUponNameDuplicate(true);
      task->setName(roiMesh->getName().c_str());

      addTask(task);
    }
  }
}

void ZRoiProvider::requestRoi(int index)
{
  if (index >= 0) {
    QMutexLocker locker(&m_roiListMutex);
    requestRoi(getSharedRoiMesh(size_t(index)));
  }
}

void ZRoiProvider::requestRoi(const std::string &name)
{
  QMutexLocker locker(&m_roiListMutex);
  requestRoi(getSharedRoiMesh(name));
}

ZMesh *ZRoiProvider::makeRoiMesh(const std::string &name)
{
  ZMesh* mesh = nullptr;

  QMutexLocker locker(&m_roiListMutex);
  ZRoiMesh *roiMesh = getRoiMesh(name);
  if (roiMesh) {
    if (roiMesh->getStatus() == ZRoiMesh::EStatus::READY) {
      mesh = new ZMesh(*roiMesh->getMesh());
    }
  } else {
    requestRoi(name);
  }

  return mesh;
}

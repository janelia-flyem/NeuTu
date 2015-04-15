#include "zmessagemanagermodel.h"
#include "zmessagemanager.h"

ZMessageManagerModel::ZMessageManagerModel(QObject *parent) :
  QAbstractItemModel(parent)
{
}

int ZMessageManagerModel::rowCount(const QModelIndex &/*parent*/) const
{
  return 0;
}

int ZMessageManagerModel::columnCount(const QModelIndex &/*parent*/) const
{
  return 2;
}

void ZMessageManagerModel::update()
{
  //removeAllRows();
  insertRow(0, QModelIndex());

  ZMessageManager &rootManager = ZMessageManager::getRootManager();

  /*
  const QObjectList &managerList = rootManager.children();
  for (size_t )

  std::vector<std::string> uuidList = m_dag.getBreadthFirstList();
  for (size_t i = 1; i < uuidList.size(); ++i) {
    std::vector<std::string> parentList = m_dag.getParentList(uuidList[i]);
    for (std::vector<std::string>::const_iterator iter = parentList.begin();
         iter != parentList.end(); ++iter) {
      insertRow(m_dag.getSiblingIndex(uuidList[i]),
                createIndex(m_dag.getSiblingIndex(*iter),0, *iter));
    }
  }
  */
}

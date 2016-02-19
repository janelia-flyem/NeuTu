#include "zdocplayerobjsmodel.h"
#include "zstackdoc.h"
#include "zobjsitem.h"

ZDocPlayerObjsModel::ZDocPlayerObjsModel(
    ZStackDoc *doc, ZStackObjectRole::TRole role, QObject *parent) :
  ZObjsModel(parent), m_doc(doc), m_role(role)
{
  updateModelData();
}

ZDocPlayerObjsModel::~ZDocPlayerObjsModel()
{

}

void ZDocPlayerObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "Type" << "Role";
  m_rootItem = new ZObjsItem(rootData, NULL);
  setupModelData(m_rootItem);
  endResetModel();
}

void ZDocPlayerObjsModel::setupModelData(ZObjsItem *parent)
{
  m_objToRow.clear();
  QList<ZDocPlayer *> playerList = m_doc->getPlayerList(m_role);

  for (int i = 0; i < playerList.size(); i++) {
    QList<QVariant> data;
    const ZDocPlayer *player = playerList.at(i);

    data << player->getTypeName() << player->getLabel();

    ZObjsItem *nodeParent = new ZObjsItem(
          data, const_cast<ZDocPlayer*>(player), parent);
    //nodeParent->setCheckState(swcTree->isVisible() ? Qt::Checked : Qt::Unchecked);
    //nodeParent->setToolTip(QString("Neuron %1: %2").arg(i + 1).arg(
    //                         QString::fromStdString(swcTree->source())));
    parent->appendChild(nodeParent);
    m_objToRow[player] = i;
  }
}

QModelIndex ZDocPlayerObjsModel::getIndex(const ZDocPlayer *stroke, int col) const
{
  std::map<const ZDocPlayer*, int>::const_iterator iter = m_objToRow.find(stroke);

  QModelIndex modelIndex;

  if (iter != m_objToRow.end()) {
    modelIndex = index(iter->second, col);
  }

  return modelIndex;
}

ZDocPlayer* ZDocPlayerObjsModel::getDocPlayer(const QModelIndex &index) const
{
  ZDocPlayer *obj = NULL;
  if (index.isValid()) {
    ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

    if (item->parent() == m_rootItem) {
      obj = static_cast<ZDocPlayer*>(item->getActuralData());
    }
  }

  return obj;
}

bool ZDocPlayerObjsModel::needCheckbox(const QModelIndex &/*index*/) const
{
  return false;
}

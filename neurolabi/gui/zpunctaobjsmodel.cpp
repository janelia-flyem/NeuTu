#include "zpunctaobjsmodel.h"

#include <QsLog.h>
#include <QFileInfo>

#include "common/utilities.h"
#include "zpunctum.h"
#include "zobjsitem.h"
#include "mvc/zstackdoc.h"
#include "neutubeconfig.h"
#include "zstackobjectinfo.h"

namespace {
// generic solution
//// partial-specialization optimization for 8-bit numbers
//template <>
//int numDigits(char n)
//{
//  // if you have the time, replace this with a static initialization to avoid
//  // the initial overhead & unnecessary branch
//  static char x[256] = {0};
//  if (x[0] == 0) {
//    for (char c = 1; c != 0; c++)
//      x[static_cast<int>(c)] = numDigits((int32_t)c);
//    x[0] = 1;
//  }
//  return x[static_cast<int>(n)];
//}
}

ZPunctaObjsModel::ZPunctaObjsModel(ZStackDoc *doc, QObject *parent) :
  ZObjsModel(parent), m_doc(doc)
{
  updateModelData();
}

ZPunctaObjsModel::~ZPunctaObjsModel()
{
}

QModelIndex ZPunctaObjsModel::getIndex(const ZPunctum *punctum, int col) const
{
  auto pun2rIt = m_punctaToRow.find(punctum);
  if (pun2rIt != m_punctaToRow.end()) {
    std::map<QString, ZObjsItem*>::const_iterator s2pIt =
        m_punctaSourceToParent.find(punctum->getSource().c_str());
    std::map<ZObjsItem*, int>::const_iterator p2rIt =
        m_punctaSourceParentToRow.find(s2pIt->second);
    return index(pun2rIt->second, col, index(p2rIt->second, 0));
  }
  return QModelIndex();
}

ZPunctum *ZPunctaObjsModel::getPunctum(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() && item->parent()->parent() == m_rootItem)
    return ZStackObject::CastVoidPointer<ZPunctum>(item->getActualData());
//    return static_cast<ZPunctum*>(item->getObj());
  else
    return NULL;
}

const std::vector<ZPunctum *> *ZPunctaObjsModel::getPuncta(const QModelIndex &index) const
{
  if (!index.isValid())
    return NULL;

  ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());

  if (item->parent() == m_rootItem) {
    std::map<ZObjsItem*, int>::const_iterator it;
    it = m_punctaSourceParentToRow.find(item);
    if (it == m_punctaSourceParentToRow.end()) {
      LERROR() << "Wrong Index";
    } else
      return &(m_punctaSeparatedByFile[it->second]);
  }

  return NULL;
}

void ZPunctaObjsModel::processObjectModified(const ZStackObjectInfoSet &infoSet)
{
  if (infoSet.hasObjectModified(ZStackObject::EType::PUNCTA) ||
      infoSet.hasObjectModified(ZStackObject::EType::PUNCTUM)) {
    updateModelData();
  }
}

void ZPunctaObjsModel::setPropertyName(const QString &key, const QString &value)
{
  m_propertyName[key] = value;
}

QString ZPunctaObjsModel::getPropertyName(const QString &key) const
{
  if (m_propertyName.count(key) > 0) {
    return m_propertyName.at(key);
  }

  return key;
}

void ZPunctaObjsModel::updateData(const ZStackObject *obj)
{
  const ZPunctum *punctum = dynamic_cast<const ZPunctum*>(obj);
  if (punctum != NULL) {
    QModelIndex index = getIndex(punctum);
    if (!index.isValid())
      return;
    ZObjsItem *item = static_cast<ZObjsItem*>(index.internalPointer());
    QList<QVariant> &data = item->getItemData();
    const ZPunctum *p = punctum;
    QList<QVariant>::iterator beginit = data.begin();
    beginit++;
    data.erase(beginit, data.end());
    data << p->score() << p->name() << p->comment() << p->x() << p->y() <<
            p->z() << p->sDevOfIntensity() << p->volSize() << p->mass() << p->radius() <<
            p->meanIntensity() << p->maxIntensity() << p->property1() << p->property2() <<
            p->property3() << p->color() << p->getSource().c_str();
    emit dataChanged(index, getIndex(punctum, item->parent()->columnCount()-1));
  }
}

void ZPunctaObjsModel::updateModelData()
{
  beginResetModel();
  delete m_rootItem;
  QList<QVariant> rootData;
  rootData << "puncta" << "score" << "name" << "comment" << "x" << "y" << "z" << "sDev" <<
              "volSize" << "mass" << "radius" << "meanIntensity" << "maxIntensity" <<
              getPropertyName("property1") << getPropertyName("property2") <<
              getPropertyName("property3") << "color" << "source";

  ZOUT(LTRACE(), 5) << "Update puncta model";
  m_rootItem = new ZObjsItem(
        rootData, &(m_doc->getObjectList(ZStackObject::EType::PUNCTUM)));
  setupModelData(m_rootItem);
  endResetModel();

  LDEBUG() << "Puncta rows:" << rowCount();
}

void ZPunctaObjsModel::setupModelData(ZObjsItem *parent)
{
  QList<QVariant> data;

  m_punctaSourceToParent.clear();
  m_punctaSourceToCount.clear();
  m_punctaToRow.clear();
  m_punctaSourceParentToRow.clear();
  m_punctaSeparatedByFile.clear();
  int sourceParentRow = 0;
  QList<ZPunctum*> punctaList = m_doc->getPunctumList();
  int numDigit = neutu::numDigits(punctaList.size()+1);
  for (int i=0; i<punctaList.size(); i++) {
    data.clear();
    ZPunctum *p = punctaList.at(i);
    QFileInfo sourceInfo(p->getSource().c_str());
    if (m_punctaSourceToParent.find(p->getSource().c_str()) !=
        m_punctaSourceToParent.end()) {
      ZObjsItem *sourceParent = m_punctaSourceToParent[p->getSource().c_str()];
      data << QString("puncta %1").
              arg(m_punctaSourceToCount[p->getSource().c_str()] + 1,
          numDigit, 10, QLatin1Char('0')) << p->score() << p->name() <<
                                             p->comment() << p->x() << p->y() <<
              p->z() << p->sDevOfIntensity() << p->volSize() << p->mass() << p->radius() <<
              p->meanIntensity() << p->maxIntensity() << p->property1() << p->property2() <<
              p->property3() << p->color() << sourceInfo.fileName();
      m_punctaToRow[p] = m_punctaSourceToCount[p->getSource().c_str()];
      m_punctaSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *punctum = new ZObjsItem(data, p, sourceParent);
      punctum->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      punctum->setToolTip(QString("puncta from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(punctum);
      m_punctaSeparatedByFile[m_punctaSourceParentToRow[sourceParent]].push_back(
            punctaList.at(i));
    } else {
      data << sourceInfo.fileName() << "score" << "name" << "comment" << "x" << "y" << "z" << "sDev" <<
              "volSize" << "mass" << "radius" << "meanIntensity" << "maxIntensity" <<
              "property1" << "property2" << "property3" << "color" << "source";
      m_punctaSeparatedByFile.push_back(std::vector<ZPunctum*>());
      ZObjsItem *sourceParent = new ZObjsItem(data, NULL, parent);
      sourceParent->setToolTip(QString("puncta source: %1").arg(p->getSource().c_str()));
      m_punctaSourceToParent[p->getSource().c_str()] = sourceParent;
      m_punctaSourceToCount[p->getSource().c_str()] = 0;
      parent->appendChild(sourceParent);
      m_punctaSourceParentToRow[sourceParent] = sourceParentRow++;

      data.clear();
      data << QString("puncta %1").arg(m_punctaSourceToCount[p->getSource().c_str()] + 1, numDigit, 10, QLatin1Char('0'))
          << p->score() << p->name() << p->comment() << p->x() << p->y() <<
             p->z() << p->sDevOfIntensity() << p->volSize() << p->mass() << p->radius() <<
             p->meanIntensity() << p->maxIntensity() << p->property1() << p->property2() <<
             p->property3() << p->color() << sourceInfo.fileName();
      m_punctaToRow[p] = m_punctaSourceToCount[p->getSource().c_str()];
      m_punctaSourceToCount[p->getSource().c_str()]++;
      ZObjsItem *punctum = new ZObjsItem(data, p, sourceParent);
      punctum->setCheckState(p->isVisible() ? Qt::Checked : Qt::Unchecked);
      punctum->setToolTip(QString("puncta from: %1").arg(p->getSource().c_str()));
      sourceParent->appendChild(punctum);
      m_punctaSeparatedByFile[m_punctaSourceParentToRow[sourceParent]].push_back(
            punctaList.at(i));
    }
  }
}

void ZPunctaObjsModel::setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs)
{
  ZObjsModel::setModelIndexCheckState(index, cs);
  if (getPunctum(index) != NULL)
    m_doc->setPunctumVisible(getPunctum(index), cs == Qt::Checked);
}

bool ZPunctaObjsModel::needCheckbox(const QModelIndex &index) const
{
  if (index.isValid()) {
    return true;
  }

  QModelIndex idx = parent(index);
  if (idx.isValid() && static_cast<ZObjsItem*>(idx.internalPointer()) == m_rootItem) {
    return true;
  }
  return getPunctum(index) != NULL;
}

#ifndef ZPUNCTAOBJSMODEL_H
#define ZPUNCTAOBJSMODEL_H

#include <map>
#include <vector>

#include "common/zsharedpointer.h"
#include "zobjsmodel.h"

class ZStackDoc;
class ZPunctum;

class ZPunctaObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZPunctaObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZPunctaObjsModel();

  QModelIndex getIndex(const ZPunctum *punctum, int col = 0) const;
  // if index is single punctum, return it, otherwise return NULL
  ZPunctum* getPunctum(const QModelIndex &index) const;
  // if index contains many puncta, return them, otherwise return NULL
  const std::vector<ZPunctum *> *getPuncta(const QModelIndex &index) const;
  void updateData(const ZStackObject *obj);

  void setPropertyName(const QString &key, const QString &value);
  QString getPropertyName(const QString &key) const;

//  void updateData(ZPunctum* punctum);

public:
  void processObjectModified(const ZStackObjectInfoSet &infoSet);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  // used to separate puncta by file (source)
  std::map<QString, ZObjsItem*> m_punctaSourceToParent;
  std::map<ZObjsItem*, int> m_punctaSourceParentToRow;
  std::map<QString, int> m_punctaSourceToCount;
  std::map<const ZPunctum*, int> m_punctaToRow;
  std::vector<std::vector<ZPunctum*> > m_punctaSeparatedByFile;
  std::map<QString, QString> m_propertyName;
  
};

#endif // ZPUNCTAOBJSMODEL_H

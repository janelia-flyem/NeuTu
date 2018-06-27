#ifndef ZROIOBJSMODEL_H
#define ZROIOBJSMODEL_H

#include <map>
#include "zobjsmodel.h"

class ZStackDoc;
class ZStackObject;
class ZStackObjectInfoSet;

class ZRoiObjsModel : public ZObjsModel
{
  Q_OBJECT

public:
  explicit ZRoiObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZRoiObjsModel();

  QModelIndex getIndex(ZStackObject* roi, int col = 0) const;
  ZStackObject* getRoi(const QModelIndex &index) const;

public slots:
  void updateModelData();
  void processObjectModified(const ZStackObjectInfoSet &infoSet);

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  std::map<ZStackObject*, int> m_roiToRow;
};

#endif // ZROIOBJSMODEL_H

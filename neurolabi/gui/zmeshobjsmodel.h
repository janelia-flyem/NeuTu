#ifndef ZMESHOBJSMODEL_H
#define ZMESHOBJSMODEL_H


#include <map>
#include "zobjsmodel.h"

class ZStackDoc;
class ZMesh;

class ZMeshObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZMeshObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZMeshObjsModel();

  QModelIndex getIndex(ZMesh* mesh, int col = 0) const;
  ZMesh* getMesh(const QModelIndex &index) const;

  void processObjectModified(const ZStackObjectInfoSet &infoSet);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  std::map<ZMesh*, int> m_meshToRow;
};

#endif // ZMESHOBJSMODEL_H

#ifndef ZSURFACEOBJSMODEL_H
#define ZSURFACEOBJSMODEL_H

#include <vector>
#include "zobjsmodel.h"

class ZStackDoc;
class ZCubeArray;

class ZSurfaceObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZSurfaceObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZSurfaceObjsModel();

  QModelIndex getIndex(ZCubeArray* cubearray, int col = 0) const;
  // if index is single punctum, return it, otherwise return NULL
  ZCubeArray* getSurface(const QModelIndex &index) const;
  // if index contains many puncta, return them, otherwise return NULL
  const std::vector<ZCubeArray*> *getSurfaceList(const QModelIndex &index) const;
  void updateData(ZCubeArray* cubearray);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  // used to separate puncta by file (source)
  std::map<QString, ZObjsItem*> m_surfaceSourceToParent;
  std::map<ZObjsItem*, int> m_surfaceSourceParentToRow;
  std::map<QString, int> m_surfaceSourceToCount;
  std::map<ZCubeArray*, int> m_surfaceToRow;
  std::vector<std::vector<ZCubeArray*> > m_surfaceSeparatedByFile;

};

#endif // ZSURFACEOBJSMODEL_H

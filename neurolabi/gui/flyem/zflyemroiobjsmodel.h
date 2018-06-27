#ifndef ZFLYEMROIOBJSMODEL_H
#define ZFLYEMROIOBJSMODEL_H

#include "zobjsmodel.h"
#include "zstackdocptr.h"

class ZStackObject;
class ZStackDoc;

class ZFlyEMRoiObjsModel : public ZObjsModel
{
  Q_OBJECT

public:
  explicit ZFlyEMRoiObjsModel(ZStackDocPtr doc, QObject *parent = 0);
  virtual ~ZFlyEMRoiObjsModel();

  QVariant data(const QModelIndex &index, int role) const;

  QModelIndex getIndex(ZStackObject* tree, int col = 0) const;
  ZStackObject* getRoi(const QModelIndex &index) const;

  ZStackDocPtr getDocument() const;

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDocPtr m_doc;
  std::map<ZStackObject*, int> m_roiToRow;
  QList<ZStackObject*> m_roiList;
};

#endif // ZFLYEMROIOBJSMODEL_H

#ifndef ZGRAPHOBJSMODEL_H
#define ZGRAPHOBJSMODEL_H

#include <vector>
#include "zobjsmodel.h"

class ZStackDoc;
class Z3DGraph;

class ZGraphObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZGraphObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZGraphObjsModel();

  QModelIndex getIndex(Z3DGraph* graph, int col = 0) const;
  // if index is single punctum, return it, otherwise return NULL
  Z3DGraph* getGraph(const QModelIndex &index) const;
  // if index contains many puncta, return them, otherwise return NULL
  const std::vector<Z3DGraph*> *getGraphList(const QModelIndex &index) const;
  void updateData(Z3DGraph* graph);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  // used to separate puncta by file (source)
  std::map<QString, ZObjsItem*> m_graphSourceToParent;
  std::map<ZObjsItem*, int> m_graphSourceParentToRow;
  std::map<QString, int> m_graphSourceToCount;
  std::map<Z3DGraph*, int> m_graphToRow;
  std::vector<std::vector<Z3DGraph*> > m_graphSeparatedByFile;

};

#endif // ZGRAPHOBJSMODEL_H

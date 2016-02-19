#ifndef ZSTROKE2DOBJSMODEL_H
#define ZSTROKE2DOBJSMODEL_H

#include <map>
#include <vector>
#include "zobjsmodel.h"

class ZStackDoc;
class ZStroke2d;

class ZStroke2dObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZStroke2dObjsModel(ZStackDoc *doc, QObject *parent = 0);
  virtual ~ZStroke2dObjsModel();

  /*!
   * \brief Get the model index of a stroke at a certain column
   */
  QModelIndex getIndex(ZStroke2d* stroke, int col = 0) const;

  ZStroke2d* getStroke2d(const QModelIndex &index) const;

  //const std::vector<ZPunctum *> *getPuncta(const QModelIndex &index) const;
  //void updateData(ZStroke2d* stroke);

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  //virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  //virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;

  // used to separate puncta by file (source)
  //std::map<QString, ZObjsItem*> m_punctaSourceToParent;
  std::map<ZObjsItem*, int> m_punctaSourceParentToRow;
  std::map<ZStroke2d*, int> m_strokeToRow;
};

#endif // ZSTROKE2DOBJSMODEL_H

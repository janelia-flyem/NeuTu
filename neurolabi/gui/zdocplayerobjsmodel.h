#ifndef ZDOCPLAYEROBJSMODEL_H
#define ZDOCPLAYEROBJSMODEL_H

#include <map>
#include "zobjsmodel.h"
#include "zdocplayer.h"

class ZStackDoc;

class ZDocPlayerObjsModel : public ZObjsModel
{
  Q_OBJECT
public:
  explicit ZDocPlayerObjsModel(
      ZStackDoc *doc, ZStackObjectRole::TRole role, QObject *parent = 0);
  virtual ~ZDocPlayerObjsModel();

  QModelIndex getIndex(const ZDocPlayer *tree, int col = 0) const;
  ZDocPlayer* getDocPlayer(const QModelIndex &index) const;

public slots:
  void updateModelData();

protected:
  void setupModelData(ZObjsItem *parent);
  //virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
  virtual bool needCheckbox(const QModelIndex &index) const;

protected:
  ZStackDoc *m_doc;
  ZStackObjectRole::TRole m_role;
  std::map<const ZDocPlayer*, int> m_objToRow;
};

#endif // ZDOCPLAYEROBJSMODEL_H

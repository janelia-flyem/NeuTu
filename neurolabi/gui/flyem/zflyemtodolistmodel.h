#ifndef ZFLYEMTODOLISTMODEL_H
#define ZFLYEMTODOLISTMODEL_H

#include <QAbstractTableModel>

#include "zsharedpointer.h"

class ZStackDoc;
class ZAbstractModelPresenter;

class ZFlyEmTodoListModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListModel(QObject *parent = 0);

//  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:

public slots:

private:
  ZSharedPointer<ZStackDoc> m_doc;
//  ZAbstractModelPresenter *m_presenter;
};

#endif // ZFLYEMTODOLISTMODEL_H

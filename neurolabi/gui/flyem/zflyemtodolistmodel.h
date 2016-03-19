#ifndef ZFLYEMTODOLISTMODEL_H
#define ZFLYEMTODOLISTMODEL_H

#include <QAbstractTableModel>

#include "zsharedpointer.h"

class ZStackDoc;

class ZFlyEmTodoListModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmTodoListModel(QObject *parent = 0);

signals:

public slots:

private:
  ZSharedPointer<ZStackDoc> m_doc;
};

#endif // ZFLYEMTODOLISTMODEL_H

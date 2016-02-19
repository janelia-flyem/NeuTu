#ifndef ZMESSAGEMANAGERMODEL_H
#define ZMESSAGEMANAGERMODEL_H

#include <QAbstractItemModel>

class ZMessageManagerModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit ZMessageManagerModel(QObject *parent = 0);

  int columnCount(const QModelIndex &parent) const;
  int rowCount(const QModelIndex &parent) const;
/*
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent= QModelIndex()) const;

  QModelIndex parent(const QModelIndex &index) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent);
*/
  void update();

signals:

public slots:

};

#endif // ZMESSAGEMANAGERMODEL_H

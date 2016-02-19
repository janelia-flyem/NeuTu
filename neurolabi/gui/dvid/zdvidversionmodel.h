#ifndef ZDVIDVERSIONMODEL_H
#define ZDVIDVERSIONMODEL_H

#include <QAbstractItemModel>
#include "zdvidversiondag.h"

class ZDvidVersionModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit ZDvidVersionModel(QObject *parent = 0);

  ~ZDvidVersionModel();

  void clear();

  int columnCount(const QModelIndex &parent) const;
  int rowCount(const QModelIndex &parent) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent= QModelIndex()) const;

  QModelIndex parent(const QModelIndex &index) const;

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent);

  std::string getVersionUuid(const QModelIndex &index) const;
  void removeAllRows();

  QModelIndex createIndex(int row, int column, const std::string &uuid) const;
  QModelIndex getIndex(const std::string &uuid) const;

  inline ZDvidVersionDag& getDag() { return m_dag; }

  void setRoot(const std::string &uuid);
  void addNode(const std::string &uuid, const std::string &parentUuid);

  ZDvidVersionDag* getDagRef() { return &m_dag; }
  void setDag(const ZDvidVersionDag &dag);

  void lockNode(const std::string &uuid);

  void activateNode(const std::string &uuid);
  void deactivateNode(const std::string &uuid);

  void update();
  void updateNodeStatus(const std::string &uuid);
  void updateNodeStatus(const QModelIndex &modelIndex);

signals:

public slots:

private:
  static std::string getUuid(qint32 id);
  static qint32 uuidToInternalId(const std::string &uuid);

private:
  ZDvidVersionDag m_dag;

};

#endif // ZDVIDVERSIONMODEL_H

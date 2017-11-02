#ifndef ZFLYEMBODYLISTMODEL_H
#define ZFLYEMBODYLISTMODEL_H

#include <QStringListModel>
#include <QSet>

class ZFlyEmBodyListModel : public QStringListModel
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyListModel(QObject *parent = 0);

  uint64_t getBodyId(int row) const;
  uint64_t getBodyId(const QModelIndex &index) const;
  int getRow(uint64_t bodyId) const;
  QModelIndex getIndex(uint64_t bodyId) const;

  void removeBody(uint64_t bodyId);

  void removeRowList(const QList<int> &rowList);

  void backupBody(int row, bool appending);

public:
  bool setData(const QModelIndex &index, const QVariant &value, int role);
  bool removeRows(int row, int count, const QModelIndex &parent);

signals:
  void bodyAdded(uint64_t bodyId);
  void bodyRemoved(uint64_t bodyId);

public slots:
  void addBody(uint64_t bodyId);

private slots:
  void processInsertedRows(const QModelIndex &modelIndex, int first, int last);
  void processChangedRows(
      const QModelIndex &topLeft, const QModelIndex &bottomRight);
//  void processRemovedRows(const QModelIndex &modelIndex, int first, int last);

private:
  void connectSignalSlot();
//  static Qt::ItemDataRole getBackupRole();

private:
  QSet<uint64_t> m_bodySet;
  QSet<uint64_t> m_backupSet;
};

#endif // ZFLYEMBODYLISTMODEL_H

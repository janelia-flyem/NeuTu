#ifndef ZSEGMENTATIONPROJECTMODEL_H
#define ZSEGMENTATIONPROJECTMODEL_H

#include <QAbstractItemModel>
#include "zsegmentationproject.h"

class ZSegmentationProjectModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ZSegmentationProjectModel(QObject *parent = 0);
  ~ZSegmentationProjectModel();

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

  void setInternalData(ZSegmentationProject *data) {
    m_data = data;
    m_currentIndex = QModelIndex();
  }

  ZTreeNode<ZObject3dScan>* getLabelNode(const QModelIndex &index) const;

  inline ZSegmentationProject* getProject()  {
    return m_data;
  }

  void generateTestData();

  void loadStack(const QString &fileName);
  void removeAllRows();

signals:

public slots:
  void loadSegmentationTarget(const QModelIndex &index);
  void updateSegmentation();

private:
  ZSegmentationProject *m_data;
  QModelIndex m_currentIndex;
};

#endif // ZSEGMENTATIONPROJECTMODEL_H

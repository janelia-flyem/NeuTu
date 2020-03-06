#ifndef ZROIITEMMODEL_H
#define ZROIITEMMODEL_H

#include <memory>

#include <QAbstractTableModel>
#include <QColor>
#include <QList>

class ZRoiProvider;

class ZRoiItemModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  ZRoiItemModel(QObject *parent = nullptr);

  enum EColumn {
    COLUMN_NAME = 0, COLUMN_COLOR, /*COLUMN_STATUS,*/ COLUMN_COUNT
  };

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(
      const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
  override;

  void setRoiProvider(const std::shared_ptr<ZRoiProvider> &roiProvider);

public:
  bool isChecked(int row) const;
  QColor getColor(int row) const;
  QString getName(int row) const;

  void setColor(int row, const QColor &color);
  void setChecked(int row, bool checked);
  void toggleChecked(const std::vector<int> &rowList);
  void setChecked(const std::vector<int> &rowList, bool checked);

signals:
  void roiRequested(int index);
//  void roiToggled(int index, bool checked);
  void roiUpdated(const QString &name, bool checked, const QColor &color);
  void roiListUpdated(
      const QList<QString> &nameList, const QList<bool> &checkedList,
      const QList<QColor> &colorList);

private:
  bool requestRoiNecessarily(int row, int column);

private slots:
  void updateModel();
  void updateRoiItem(int row);
  void updateRoiItem(int row, int column);
  void updateRoi(int row);
  void updateRoiItem(const QString &name);

private:
  std::shared_ptr<ZRoiProvider> m_roiProvider;
  //necessary for independent views
  std::vector<bool> m_visibleList;
  std::vector<QColor> m_colorList;
};

#endif // ZROIITEMMODEL_H

#ifndef ZROIITEMMODEL_H
#define ZROIITEMMODEL_H

#include <memory>

#include <QAbstractItemModel>
#include <QColor>

class ZRoiProvider;

class ZRoiItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  ZRoiItemModel(QObject *parent = nullptr);

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(
      const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
  override;

  void setRoiProvider(const std::shared_ptr<ZRoiProvider> &roiProvider);


private slots:
  void updateModel();

private:
  std::shared_ptr<ZRoiProvider> m_roiProvider;
  std::vector<bool> m_visibleList;
  std::vector<QColor> m_colorList;
};

#endif // ZROIITEMMODEL_H

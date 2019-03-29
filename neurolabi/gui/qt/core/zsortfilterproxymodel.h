#ifndef ZSORTFILTERPROXYMODEL_H
#define ZSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/*!
 * \brief The class for enhancing QSortFilterProxyModel
 *
 * Currently it is used to hide rows with empty data at its first column
 */
class ZSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  explicit ZSortFilterProxyModel(QObject *parent = nullptr);

  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

//  void update();

signals:

public slots:
};

#endif // ZSORTFILTERPROXYMODEL_H

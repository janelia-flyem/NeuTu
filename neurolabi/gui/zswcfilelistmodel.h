#ifndef ZSWCFILELISTMODEL_H
#define ZSWCFILELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QFileInfoList>

class ZSwcFileListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit ZSwcFileListModel(QObject *parent = 0);

  void loadDir(const QString &dirPath, bool removingExtra);

  static QFileInfoList LoadDir(const QString &dirPath, bool removingExtra);

  int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const;

  void deleteAll();
  void deleteFile(int index);

  bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

  QString getFilePath(int index);

  int getMaxFileCount() const;

signals:

public slots:

private:
  QFileInfoList m_fileList;
  static int maxFileCount;
};

#endif // ZSWCFILELISTMODEL_H

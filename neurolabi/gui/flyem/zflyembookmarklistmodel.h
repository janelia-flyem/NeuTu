#ifndef ZFLYEMBOOKMARKLISTMODEL_H
#define ZFLYEMBOOKMARKLISTMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QString>
#include "flyem/zflyembookmark.h"
#include "flyem/zflyembookmarkpresenter.h"
//#include "flyem/zflyembookmarkarray.h"
#include "flyem/zflyembookmarkptrarray.h"

class ZFlyEmBookmarkListModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit ZFlyEmBookmarkListModel(QObject *parent = 0);

  int rowCount( const QModelIndex & parent = QModelIndex() ) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool insertColumns(int col, int count, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
  bool removeColumns(int col, int count, const QModelIndex &parent = QModelIndex());

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const;

  void clear();
  void append(const ZFlyEmBookmark *bookmark);
  void update(int row);

  const ZFlyEmBookmark* getBookmark(int index) const;
  ZFlyEmBookmark* getBookmark(int index);
  const ZFlyEmBookmarkPtrArray& getBookmarkArray() const;

  void removeBookmark(ZFlyEmBookmark *bookmark);

//  void appendBookmark(const ZFlyEmBookmark &bookmark);

  //void load(const QString &filePath);

signals:

public slots:

private:
  ZFlyEmBookmarkPtrArray m_bookmarkArray;
//  ZFlyEmBookmarkArray m_bookmarkArray;
  ZFlyEmBookmarkPresenter *m_presenter;
  ZFlyEmBookmarkPresenter *m_defaultPresenter;
};

#endif // ZFLYEMBOOKMARKLISTMODEL_H

#ifndef ZROILISTVIEW_H
#define ZROILISTVIEW_H

#include <QTreeView>
#include "zstackdocptr.h"

class ZFlyEMRoiObjsModel;

class ZRoiListView : public QTreeView
{
  Q_OBJECT
public:
  explicit ZRoiListView(ZStackDocPtr doc, QWidget *parent = nullptr);
  ~ZRoiListView();

signals:

public slots:

protected:

private:
  void init();

private:
  ZFlyEMRoiObjsModel *m_model = NULL;
};

#endif // ZROILISTVIEW_H

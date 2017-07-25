#ifndef ZFLYEMBODYLISTVIEW_H
#define ZFLYEMBODYLISTVIEW_H

#include <QListView>

class ZFlyEmBodyListModel;

class ZFlyEmBodyListView : public QListView
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyListView(QWidget *parent = 0);

  ZFlyEmBodyListModel* getModel() const;

  QSet<uint64_t> getSelectedSet() const;

signals:
  void bodySelectionChanged(QSet<uint64_t> selectedSet);

public slots:

private slots:
  void processSelectionChange();

};

#endif // ZFLYEMBODYLISTVIEW_H

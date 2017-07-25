#ifndef ZBODYLISTWIDGET_H
#define ZBODYLISTWIDGET_H

#include <QWidget>

namespace Ui {
class ZBodyListWidget;
}

class ZFlyEmBodyListModel;

class ZBodyListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZBodyListWidget(QWidget *parent = 0);
  ~ZBodyListWidget();

  ZFlyEmBodyListModel *getModel() const;

signals:
  void bodyAdded(uint64_t bodyId);
  void bodyRemoved(uint64_t bodyId);
  void bodySelectionChanged(QSet<uint64_t> selectedSet);

private slots:
  void addString();
  void removeSelectedString();
  void processBodySelectionChange(const QSet<uint64_t> &selectedSet);

private:
  Ui::ZBodyListWidget *ui;
};

#endif // ZBODYLISTWIDGET_H

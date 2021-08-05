#ifndef ZBODYLISTWIDGET_H
#define ZBODYLISTWIDGET_H

#include <QWidget>

namespace Ui {
class ZBodyListWidget;
}

class ZFlyEmBodyListModel;
class ZFlyEmBodyListView;

/*!
 * \brief The widget class for exploring neuron bodies in DVID
 */
class ZBodyListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZBodyListWidget(QWidget *parent = 0);
  ~ZBodyListWidget();

  /*!
   * \brief Get the list view of containing bodies to explore
   *
   * \return The pointer to the list view.
   *         It is guaranteed to be a non-NULL pointer.
   */
  ZFlyEmBodyListView* getView() const;

  /*!
   * \brief Get the list model of containing bodies to explore
   *
   * \return The pointer to the list model.
   *         It is guaranteed to be a non-NULL pointer.
   */
  ZFlyEmBodyListModel* getModel() const;

public slots:
  void selectBodyItem(uint64_t bodyId);
  void deselectBodyItem(uint64_t bodyId);

  void selectBodyItemSliently(uint64_t bodyId);
  void deselectBodyItemSliently(uint64_t bodyId);

  void removeBody(uint64_t bodyId);
  void addBody(uint64_t bodyId);
  void addBodies(QList<uint64_t> bodyList);

  void diagnose();

signals:
  void bodyAdded(uint64_t bodyId);
  void bodyRemoved(uint64_t bodyId);
  void bodyItemSelectionChanged(QSet<uint64_t> selectedSet);

private slots:
  void addString();
  void removeSelectedString();
  void processBodySelectionChange(const QSet<uint64_t> &selectedSet);

private:
  Ui::ZBodyListWidget *ui;
};

#endif // ZBODYLISTWIDGET_H

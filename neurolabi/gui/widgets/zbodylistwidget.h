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
   * \return The pointer to the list view.
   *         It is guaranteed to be a non-NULL pointer.
   */
  ZFlyEmBodyListView* getView() const;

  /*!
   * \brief Get the list model of containing bodies to explore
   * \return The pointer to the list model.
   *         It is guaranteed to be a non-NULL pointer.
   */
  ZFlyEmBodyListModel* getModel() const;

public slots:
  void selectBody(uint64_t bodyId);
  void deselectBody(uint64_t bodyId);

  void selectBodySliently(uint64_t bodyId);
  void deselectBodySliently(uint64_t bodyId);

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

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

  void setBodySelection(uint64_t bodyId, bool selected);

  /*!
   * \brief Set body selection silently
   *
   * No message of the selection change will be sent outside of the class.
   *
   * \param bodyId The ID of the body for selection
   * \param selected Select or deselect the body
   */
  void setBodySelectionSliently(uint64_t bodyId, bool selected);

  void setIndexSelectionSliently(const QModelIndex &index, bool selected);

signals:
  void bodyItemSelectionChanged(QSet<uint64_t> selectedSet);

public slots:

private slots:
  void processSelectionChange();

private:
  void setBodySelection(uint64_t bodyId, bool selected, bool silent);
  void setIndexSelection(const QModelIndex &index, bool selected, bool silent);
};

#endif // ZFLYEMBODYLISTVIEW_H

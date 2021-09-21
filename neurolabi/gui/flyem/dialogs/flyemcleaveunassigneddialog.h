#ifndef FLYEMCLEAVEUNASSIGNEDDIALOG_H
#define FLYEMCLEAVEUNASSIGNEDDIALOG_H

#include <QDialog>

namespace Ui {
class FlyEmCleaveUnassignedDialog;
}

class FlyEmCleaveUnassignedDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmCleaveUnassignedDialog(QWidget *parent = nullptr);
  ~FlyEmCleaveUnassignedDialog();

  enum class EOption {
    NONE, MAIN_BODY, NEW_BODY
  };

  void setUnassignedCount(size_t n);
  void setIndexNotCleavedOff(size_t index);
  EOption getOption() const;

  void reset();

private slots:
  void updateWidget();

private:
  Ui::FlyEmCleaveUnassignedDialog *ui;
  size_t m_unassignedCount = 0;
  size_t m_indexNotCleavedOff = 0;
};

#endif // FLYEMCLEAVEUNASSIGNEDDIALOG_H

#ifndef FLYEMBODYFILTERDIALOG_H
#define FLYEMBODYFILTERDIALOG_H

#include <QDialog>
#include <vector>
#include <set>

namespace Ui {
class FlyEmBodyFilterDialog;
}

class FlyEmBodyFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FlyEmBodyFilterDialog(QWidget *parent = 0);
  ~FlyEmBodyFilterDialog();

  size_t getMinBodySize() const;
  size_t getMaxBodySize() const;
  bool hasUpperBodySize() const;

  std::vector<int> getExcludedBodies() const;
  std::set<int> getExcludedBodySet() const;

private:
  Ui::FlyEmBodyFilterDialog *ui;
};

#endif // FLYEMBODYFILTERDIALOG_H

#ifndef FLYEMBODYFILTERDIALOG_H
#define FLYEMBODYFILTERDIALOG_H

#include <QDialog>
#include <vector>
#include <set>
#include "dvid/zdvidfilter.h"

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
  bool namedBodyOnly() const;
  bool tracedOnly() const;

  QString getBodyListFile() const;
  void setBodyListFile(const QString path);

  std::vector<uint64_t> getExcludedBodies() const;
  std::set<uint64_t> getExcludedBodySet() const;

  std::vector<uint64_t> getBodyIdArray() const;

  ZDvidFilter getDvidFilter() const;

public slots:
  void setBodyListFile();

private:
  Ui::FlyEmBodyFilterDialog *ui;
};

#endif // FLYEMBODYFILTERDIALOG_H

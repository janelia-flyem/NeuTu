#ifndef DVIDSKELETONIZEDIALOG_H
#define DVIDSKELETONIZEDIALOG_H

#include <QDialog>
#include "zdviddialog.h"
#include "dvid/zdvidtarget.h"
#include "flyembodyfilterdialog.h"

namespace Ui {
class DvidSkeletonizeDialog;
}

class DvidSkeletonizeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DvidSkeletonizeDialog(QWidget *parent = 0);
  ~DvidSkeletonizeDialog();

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int getMaxBodySize() const;
  int getMinBodySize() const;
  bool hasUpperBodySize() const;
  std::set<int> getExcludedBodySet() const;

  bool noOverwriting() const;

  //void setBodyFilter(const FlyEmBodyFilterDialog &m_bodyDlg);

public slots:
  void setDvidTarget();
  void selectBody();

private:
  void updateDvidTarget();
  void updateBodySelection();

private:
  Ui::DvidSkeletonizeDialog *ui;
  ZDvidDialog *m_dvidDlg;
  ZDvidTarget m_dvidTarget;
  FlyEmBodyFilterDialog *m_bodyDlg;
};

#endif // DVIDSKELETONIZEDIALOG_H

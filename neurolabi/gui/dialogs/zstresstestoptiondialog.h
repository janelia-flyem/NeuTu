#ifndef ZSTRESSTESTOPTIONDIALOG_H
#define ZSTRESSTESTOPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ZStressTestOptionDialog;
}

class ZStressTestOptionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZStressTestOptionDialog(QWidget *parent = 0);
  ~ZStressTestOptionDialog();

  enum EOption {
    OPTION_CUSTOM, OPTION_BODY_MERGE, OPTION_BODY_SPLIT, OPTION_BODY_3DVIS,
    OPTION_OBJECT_MANAGEMENT
  };

  EOption getOption() const;

private:
  Ui::ZStressTestOptionDialog *ui;
};

#endif // ZSTRESSTESTOPTIONDIALOG_H

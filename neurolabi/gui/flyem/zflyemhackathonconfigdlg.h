#ifndef ZFLYEMHACKATHONCONFIGDLG_H
#define ZFLYEMHACKATHONCONFIGDLG_H

#include <QDialog>

namespace Ui {
class ZFlyEmHackathonConfigDlg;
}

class ZFlyEmHackathonConfigDlg : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmHackathonConfigDlg(QWidget *parent = 0);
  ~ZFlyEmHackathonConfigDlg();

  QString getSourceDir() const;
  QString getWorkDir() const;
  bool usingInternalDvid() const;

private slots:
  QString setWorkDir();
  QString setSourceDir();

private:
  Ui::ZFlyEmHackathonConfigDlg *ui;
};

#endif // ZFLYEMHACKATHONCONFIGDLG_H

#ifndef ZDVIDDIALOG_H
#define ZDVIDDIALOG_H

#include <QDialog>
#include <string>
#include "dvid/zdvidtarget.h"

namespace Ui {
class ZDvidDialog;
}

class ZDvidDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZDvidDialog(QWidget *parent = 0);
  ~ZDvidDialog();

  void loadConfig(const std::string &filePath);

  int getPort() const;
  QString getAddress() const;
  QString getUuid() const;

  ZDvidTarget getDvidTarget() const;

public slots:
  void setServer(int index);

private:
  Ui::ZDvidDialog *ui;
  std::vector<ZDvidTarget> m_dvidRepo;
  std::string m_customString;
  const static char *m_dvidRepoKey;
};

#endif // ZDVIDDIALOG_H

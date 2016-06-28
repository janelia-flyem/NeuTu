#ifndef STRINGLISTDIALOG_H
#define STRINGLISTDIALOG_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class StringListDialog;
}

class StringListDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StringListDialog(QWidget *parent = 0);
  ~StringListDialog();

  QStringListModel* getModel() const {
    return m_model;
  }

  void addString(const QString &str);
  void setStringList(const QStringList &list);
  void setStringList(const std::vector<std::string> &data);
  QStringList getStringList() const;

public slots:
  void addString();
  void removeSelectedString();
  void removeAllString();

private:
  Ui::StringListDialog *ui;
  QStringListModel *m_model;
};

#endif // STRINGLISTDIALOG_H

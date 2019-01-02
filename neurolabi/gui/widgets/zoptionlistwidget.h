#ifndef ZOPTIONLISTWIDGET_H
#define ZOPTIONLISTWIDGET_H

#include <QWidget>

namespace Ui {
class ZOptionListWidget;
}

class ZOptionListWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ZOptionListWidget(QWidget *parent = 0);
  ~ZOptionListWidget();

  void setOptionList(const QStringList &strList);

  QStringList getSelectedOptionList() const;
  void setName(const QString &name);

private slots:
  void addSelectedOption();

private:
  Ui::ZOptionListWidget *ui;
  QString m_delimiter = ";";
};

#endif // ZOPTIONLISTWIDGET_H

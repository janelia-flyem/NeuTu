#ifndef ZCOMBOEDITWIDGET_H
#define ZCOMBOEDITWIDGET_H

#include <string>
#include <vector>

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>

class ZComboEditWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ZComboEditWidget(QWidget *parent = 0);

  void setStringList(const QStringList &stringList);
  void setStringList(const std::vector<std::string> &stringList);

  QString getText() const;

  void setCurrentIndex(int index);

signals:

public slots:
  void setEditText();
  void resetComboBox();

private:
  void connectSignalSlot();

private:
  QComboBox *m_comboBox;
  QLineEdit *m_textEdit;
};

#endif // ZCOMBOEDITWIDGET_H

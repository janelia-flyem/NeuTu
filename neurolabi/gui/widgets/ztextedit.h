#ifndef ZTEXTEDIT_H
#define ZTEXTEDIT_H

#include <QTextEdit>

class ZTextEdit : public QTextEdit
{
  Q_OBJECT
public:
  explicit ZTextEdit(QWidget *parent = 0);

protected:
//  void paintEvent(QPaintEvent *e);

signals:

public slots:
};

#endif // ZTEXTEDIT_H

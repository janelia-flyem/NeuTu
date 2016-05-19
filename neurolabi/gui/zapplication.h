#ifndef ZAPPLICATION_H
#define ZAPPLICATION_H

#include <QApplication>

class ZApplication : public QApplication
{
  Q_OBJECT
public:
  explicit ZApplication(int argc, char **argv, bool GUIenabled);

  bool notify(QObject *, QEvent *);

signals:

public slots:

};

#endif // ZAPPLICATION_H

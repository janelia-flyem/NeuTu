#ifndef ZDIALOGFACTORY_H
#define ZDIALOGFACTORY_H

#include <QObject>

class ZDvidDialog;

class ZDialogFactory
{
public:
  ZDialogFactory();

  static ZDvidDialog* makeDvidDialog(QWidget *parent = 0);
};

#endif // ZDIALOGFACTORY_H

#ifndef FLYEMDIALOGFACTORY_H
#define FLYEMDIALOGFACTORY_H

#include <QWidget>

class ZFlyEmBodyAnnotationDialog;
class ZFlyEmProofDoc;

class FlyEmDialogFactory
{
public:
  FlyEmDialogFactory();

  static ZFlyEmBodyAnnotationDialog* MakeBodyAnnotationDialog(
      ZFlyEmProofDoc *doc, QWidget *parent);
};


#endif // FLYEMDIALOGFACTORY_H

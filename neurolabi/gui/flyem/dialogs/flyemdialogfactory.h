#ifndef FLYEMDIALOGFACTORY_H
#define FLYEMDIALOGFACTORY_H

#include <QWidget>

//class ZFlyEmBodyAnnotationDialog;
class ZFlyEmProofDoc;
class FlyEmBodyAnnotationDialog;
class ZGenericBodyAnnotationDialog;
class ZJsonObject;

class FlyEmDialogFactory
{
public:
  FlyEmDialogFactory();

//  static ZFlyEmBodyAnnotationDialog* MakeBodyAnnotationDialog(
//      ZFlyEmProofDoc *doc, QWidget *parent);
  static FlyEmBodyAnnotationDialog* MakeBodyAnnotationDialog(
      ZFlyEmProofDoc *doc, QWidget *parent);
  static ZGenericBodyAnnotationDialog* MakeBodyAnnotaitonDialog(
      ZFlyEmProofDoc *doc, ZJsonObject config, QWidget *parent);
};


#endif // FLYEMDIALOGFACTORY_H

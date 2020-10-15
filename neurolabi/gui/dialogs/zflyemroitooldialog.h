#ifndef ZFLYEMROITOOLDIALOG_H
#define ZFLYEMROITOOLDIALOG_H

#include <QDialog>
#include <QList>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"

namespace Ui {
class ZFlyEmRoiToolDialog;
}

class ZFlyEmProofMvc;
class ZFlyEmProofDoc;
class ZFlyEmRoiProject;
class ZWidgetMessage;

class ZFlyEmRoiToolDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmRoiToolDialog(QWidget *parent = 0);
  ~ZFlyEmRoiToolDialog();

  ZFlyEmProofMvc* getParentFrame() const;
  ZFlyEmProofDoc* getDocument() const;

  bool isActive() const;
  void clear();

  ZFlyEmRoiProject* getProject() const {
    return m_project;
  }

  bool appendProject(ZFlyEmRoiProject *project);

  void updateDvidTarget();
  void downloadAllProject();

  void goToNearestRoiSlice(int z);

  void updateRoi();

public slots:
  void openProject(int index);
  void newProject();
  void cloneProject();
  void dump(const ZWidgetMessage &msg);
  void dump(const QString &msg);
  void processMessage(const ZWidgetMessage &msg);
  void uploadRoi();
  void prevSlice();
  void nextSlice();
  void estimateRoi();
  void createRoiData();
  void exportRoiData();
  void estimateRoiVolume();
  void movePlaneLeft();
  void movePlaneRight();
  void movePlaneUp();
  void movePlaneDown();
  void movePlaneLeftUp();
  void movePlaneRightDown();
  void rotatePlaneClockwise();
  void rotatePlaneCounterClockwise();
  void rotatePlane(double theta);
  void movePlane(double dx, double dy);
  void scalePlane(double sx, double sy);
  void expandPlane();
  void shrinkPlane();
  void expandPlaneX();
  void shrinkPlaneX();
  void expandPlaneY();
  void shrinkPlaneY();


signals:
  void projectActivited();
  void projectClosed();
  void showing3DRoiCurve();
  void steppingSlice(int);
  void goingToSlice(int);
  void goingToNearestRoi();
  void estimatingRoi();
//  void movingPlaneLeft();
  void movingPlane(double dx, double dy);
  void rotatingPlane(double theta);
  void scalingPlane(double sx, double sy);

private:
  void init();
  ZFlyEmRoiProject* newProject(const QString &name);
  ZFlyEmRoiProject* newProjectWithoutCheck(const QString &name);
  void cloneProject(const QString &name);
  bool isValidName(const QString &name) const;
  void uploadProjectList();
  int getSliceStep() const;

  double getMoveStep() const;
  double getRotateStep() const;
  double getScaleStep() const;

private:
  Ui::ZFlyEmRoiToolDialog *ui;
  QList<ZFlyEmRoiProject*> m_projectList;
  ZFlyEmRoiProject *m_project;
  ZDvidReader m_dvidReader;
  ZDvidWriter m_dvidWriter;
};

#endif // ZFLYEMROITOOLDIALOG_H

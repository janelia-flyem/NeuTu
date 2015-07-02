#ifndef FLYEMPROOFCONTROLFORM_H
#define FLYEMPROOFCONTROLFORM_H

#include <QWidget>
#include "tz_stdint.h"
#include "zflyembookmarklistmodel.h"

class QMenu;
class ZDvidTarget;
class ZFlyEmBodyMergeProject;

namespace Ui {
class FlyEmProofControlForm;
}

class FlyEmProofControlForm : public QWidget
{
  Q_OBJECT

public:
  explicit FlyEmProofControlForm(QWidget *parent = 0);
  ~FlyEmProofControlForm();

signals:
  void segmentVisibleChanged(bool visible);
  void mergingSelected();
  void dvidSetTriggered();
  void splitTriggered(uint64_t bodyId);
  void splitTriggered();
  void labelSizeChanged(int width, int height);
  void coarseBodyViewTriggered();
  void savingMerge();
  void committingMerge();
  void zoomingTo(int x, int y, int z);
  void locatingBody(uint64_t);
  void goingToBody();
  void selectingBody();

public slots:
  void setInfo(const QString &info);
  void setDvidInfo(const ZDvidTarget &target);
  void updateBookmarkTable(ZFlyEmBodyMergeProject *project);

private slots:
  void setSegmentSize();
  void incSegmentSize();
  void decSegmentSize();
  void goToPosition();
  void goToBody();
  void selectBody();
  void locateBookmark(const QModelIndex &index);

private:
  void createMenu();

private:
  Ui::FlyEmProofControlForm *ui;
  QMenu *m_mainMenu;
  ZFlyEmBookmarkListModel m_bookmarkList;

//  ZDvidDialog *m_dvidDlg;
};

#endif // FLYEMPROOFCONTROLFORM_H

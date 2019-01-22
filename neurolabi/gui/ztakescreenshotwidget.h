#ifndef ZTAKESCREENSHOTWIDGET_H
#define ZTAKESCREENSHOTWIDGET_H

#include "z3dgl.h"
#include "widgets/znumericparameter.h"
#include "widgets/zoptionparameter.h"
#include "widgets/zstringparameter.h"
#include <QWidget>
#include <QScrollArea>
#include <QDir>

class QPushButton;

class QRadioButton;

class QGroupBox;

class ZSelectFileWidget;

class ZTakeScreenShotWidget : public QScrollArea
{
Q_OBJECT
public:
  explicit ZTakeScreenShotWidget(bool is2D = false, bool group = false, QWidget* parent = nullptr);

  // In stereo rendering mode, we can only capture stereo image.
  // In normal rendering mode or if stereo is not supported by graphic card,
  // we can check this checkbox to capture stereo image, or not to capture normal image
  void setCaptureStereoImage(bool v)
  {
    m_captureStereoImage.set(v);
    if (v) m_captureStereoImage.setVisible(false);
  }

signals:

  void takeFixedSize2DScreenShot(const QString& filename, int width, int height);

  void take2DScreenShot(const QString& filename);

  void takeFixedSize3DScreenShot(const QString& filename, int width, int height, Z3DScreenShotType sst);

  void take3DScreenShot(const QString& filename, Z3DScreenShotType sst);

  void takeSeriesFixedSize3DScreenShot(const QDir& dir, const QString& namePrefix, glm::vec3 axis,
                                       bool clockWise, int numFrame, int width, int height, Z3DScreenShotType sst);

  void takeSeries3DScreenShot(const QDir& dir, const QString& namePrefix, glm::vec3 axis,
                              bool clockWise, int numFrame, Z3DScreenShotType sst);

protected:
  void captureButtonPressed();

  void captureSequenceButtonPressed();

  void setFileNameSource();

  void updateImageSizeWidget();

  void prefixChanged();

  void adjustWidget();

private:
  void createWidget();

private:
  bool m_group;

  QGroupBox* m_groupBox;

  ZStringIntOptionParameter m_mode;
  ZBoolParameter m_captureStereoImage;
  ZStringIntOptionParameter m_stereoImageType;
  ZBoolParameter m_useWindowSize;
  ZIVec2Parameter m_customSize;

  QRadioButton* m_useManualName;
  QRadioButton* m_useAutoName;

  ZSelectFileWidget* m_folderWidget;
  ZStringParameter m_namePrefix;
  QPushButton* m_captureButton;

  QString m_lastFName;
  int m_nextNumber;

  ZStringIntOptionParameter m_axis;
  ZBoolParameter m_clockwise;
  ZIntParameter m_timeInSecond;
  ZIntParameter m_framePerSecond;
  QPushButton* m_captureSequenceButton;

  bool m_is2D;
};

#endif // ZTAKESCREENSHOTWIDGET_H

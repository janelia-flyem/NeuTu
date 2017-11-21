#include "ztakescreenshotwidget.h"

#include "zselectfilewidget.h"
#include "z3dgpuinfo.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QPushButton>
#include <QRadioButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QSettings>
#include <QApplication>

ZTakeScreenShotWidget::ZTakeScreenShotWidget(bool is2D, bool group, QWidget* parent)
  : QScrollArea(parent)
  , m_group(group)
  , m_mode("Mode")
  , m_captureStereoImage("Capture Stereo Image", false)
  , m_stereoImageType("Stereo Image Type")
  , m_useWindowSize("Use Window Size as Image Size", true)
  , m_customSize("Use Custom Image Size", glm::ivec2(1920, 1080), glm::ivec2(128, 128),
                 glm::ivec2(16000, 16000))
  , m_namePrefix("filename prefix", "")
  , m_nextNumber(1)
  , m_axis("Rotate Around Axis")
  , m_clockwise("Clockwise Rotate", false)
  , m_timeInSecond("Total Time (seconds)", 30, 1, 600)
  , m_framePerSecond("Frame per Second", 30, 24, 60)
  , m_is2D(is2D)
{
  m_customSize.setStyle("SPINBOX");
  QList<QString> names;
  names.push_back("Width:");
  names.push_back("Height:");
  m_customSize.setNameForEachValue(names);
  m_mode.addOptions("Capture Single Image", "Capture Rotating Image sequence");
  m_mode.select("Capture Single Image");
  m_stereoImageType.addOptions("Full Side-By-Side", "Half Side-By-Side");
  m_stereoImageType.select("Full Side-By-Side");
  m_axis.addOptions("X", "Y", "Z");
  m_axis.select("Y");
  QDateTime data = QDateTime::currentDateTime();
  QString prefix = QString("neuTubeCapture") + data.toString("yyyyMMdd") + QString("_");
  m_namePrefix.set(prefix);
  createWidget();
  connect(&m_mode, &ZStringIntOptionParameter::valueChanged, this, &ZTakeScreenShotWidget::adjustWidget);
  connect(&m_captureStereoImage, &ZBoolParameter::valueChanged, this, &ZTakeScreenShotWidget::adjustWidget);
  connect(&m_useWindowSize, &ZBoolParameter::valueChanged, this, &ZTakeScreenShotWidget::updateImageSizeWidget);
  connect(&m_namePrefix, &ZStringParameter::valueChanged, this, &ZTakeScreenShotWidget::prefixChanged);
  connect(m_captureButton, &QPushButton::clicked, this, &ZTakeScreenShotWidget::captureButtonPressed);
  connect(m_captureSequenceButton, &QPushButton::clicked, this, &ZTakeScreenShotWidget::captureSequenceButtonPressed);
  adjustWidget();
  updateImageSizeWidget();
}

void ZTakeScreenShotWidget::captureButtonPressed()
{
  QString filepath;
  if (m_useManualName->isChecked()) {
    filepath = QFileDialog::getSaveFileName(
      this, "Save Capture", m_lastFName, "",
      nullptr/*, QFileDialog::DontUseNativeDialog*/);
    if (!filepath.isEmpty()) {
      m_lastFName = filepath;
    }
  } else {  // auto generate file name based on folder and prefix
    if (m_folderWidget->getSelectedDirectory().isEmpty()) {
      QMessageBox::critical(this, qApp->applicationName(), "Output Folder do not exist");
      return;
    }
    QDir dir(m_folderWidget->getSelectedDirectory());

    QSettings settings;
    settings.setValue(QString("ScreenShot/exportPath"), dir.absolutePath());

    while (true) {
      QString filename = QString("%1%2.tif").arg(m_namePrefix.get()).arg(m_nextNumber++);
      if (dir.exists(filename))
        continue;
      filepath = dir.filePath(filename);
      break;
    }
  }

  if (m_is2D) {
    if (m_useWindowSize.get())
      emit take2DScreenShot(filepath);
    else {
      glm::ivec2 size = m_customSize.get();
      emit takeFixedSize2DScreenShot(filepath, size.x, size.y);
    }
  } else {
    Z3DScreenShotType sst;
    if (m_captureStereoImage.get()) {
      if (m_stereoImageType.isSelected("Half Side-By-Side"))
        sst = Z3DScreenShotType::HalfSideBySideStereoView;
      else
        sst = Z3DScreenShotType::FullSideBySideStereoView;
    } else
      sst = Z3DScreenShotType::MonoView;

    if (m_useWindowSize.get())
      emit take3DScreenShot(filepath, sst);
    else {
      glm::ivec2 size = m_customSize.get();
      emit takeFixedSize3DScreenShot(filepath, size.x, size.y, sst);
    }
  }
}

void ZTakeScreenShotWidget::captureSequenceButtonPressed()
{
  if (m_folderWidget->getSelectedDirectory().isEmpty()) {
    QMessageBox::critical(this, qApp->applicationName(), "Output Folder do not exist");
    return;
  }
  QDir dir(m_folderWidget->getSelectedDirectory());
  glm::vec3 axis;
  if (m_axis.isSelected("X"))
    axis = glm::vec3(1.f, 0.f, 0.f);
  else if (m_axis.isSelected("Y"))
    axis = glm::vec3(0.f, 1.f, 0.f);
  else
    axis = glm::vec3(0.f, 0.f, 1.f);
  int numFrame = m_framePerSecond.get() * m_timeInSecond.get();

  Z3DScreenShotType sst;
  if (m_captureStereoImage.get()) {
    if (m_stereoImageType.isSelected("Half Side-By-Side"))
      sst = Z3DScreenShotType::HalfSideBySideStereoView;
    else
      sst = Z3DScreenShotType::FullSideBySideStereoView;
  } else
    sst = Z3DScreenShotType::MonoView;

  if (m_useWindowSize.get())
    emit takeSeries3DScreenShot(dir, m_namePrefix.get(), axis, m_clockwise.get(), numFrame, sst);
  else {
    glm::ivec2 size = m_customSize.get();
    emit takeSeriesFixedSize3DScreenShot(dir, m_namePrefix.get(), axis, m_clockwise.get(), numFrame,
                                         size.x, size.y, sst);
  }
}

void ZTakeScreenShotWidget::setFileNameSource()
{
  if (m_useManualName->isChecked()) {
    m_folderWidget->setEnabled(false);
    m_namePrefix.setEnabled(false);
  } else {
    m_folderWidget->setEnabled(true);
    m_namePrefix.setEnabled(true);
  }
}

void ZTakeScreenShotWidget::updateImageSizeWidget()
{
  if (m_useWindowSize.get())
    m_customSize.setEnabled(false);
  else
    m_customSize.setEnabled(true);
}

void ZTakeScreenShotWidget::prefixChanged()
{
  m_nextNumber = 1;
}

void ZTakeScreenShotWidget::adjustWidget()
{
  if (m_mode.isSelected("Capture Single Image")) {
    m_useAutoName->setVisible(true);
    m_useManualName->setVisible(true);
    m_captureButton->setVisible(true);
    m_axis.setVisible(false);
    m_clockwise.setVisible(false);
    m_timeInSecond.setVisible(false);
    m_framePerSecond.setVisible(false);
    m_captureSequenceButton->setVisible(false);
    setFileNameSource();
  } else {
    m_useAutoName->setVisible(false);
    m_useManualName->setVisible(false);
    m_captureButton->setVisible(false);
    m_axis.setVisible(true);
    m_clockwise.setVisible(true);
    m_timeInSecond.setVisible(true);
    m_framePerSecond.setVisible(true);
    m_captureSequenceButton->setVisible(true);
    m_folderWidget->setEnabled(true);
    m_namePrefix.setEnabled(true);
  }
  m_stereoImageType.setVisible(m_captureStereoImage.get());
}

void ZTakeScreenShotWidget::createWidget()
{
  auto lo = new QVBoxLayout;

  QHBoxLayout* hlo = nullptr;
  QWidget* wg = nullptr;
  QLabel* label = nullptr;

  if (!m_is2D) {
    hlo = new QHBoxLayout;
    label = m_mode.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_mode.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);

    hlo = new QHBoxLayout;
    label = m_captureStereoImage.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_captureStereoImage.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);

    hlo = new QHBoxLayout;
    label = m_stereoImageType.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_stereoImageType.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);
  }

  hlo = new QHBoxLayout;
  label = m_useWindowSize.createNameLabel();
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  label->setMinimumWidth(125);
  //label->setWordWrap(true);
  hlo->addWidget(label);
  wg = m_useWindowSize.createWidget();
  wg->setMinimumWidth(175);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  label = m_customSize.createNameLabel();
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  label->setMinimumWidth(125);
  //label->setWordWrap(true);
  hlo->addWidget(label);
  wg = m_customSize.createWidget();
  wg->setMinimumWidth(175);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  m_useManualName = new QRadioButton("Manual specify output name", this);
  connect(m_useManualName, &QRadioButton::clicked, this, &ZTakeScreenShotWidget::setFileNameSource);
  lo->addWidget(m_useManualName);
  m_useAutoName = new QRadioButton("Auto generate output name", this);
  connect(m_useAutoName, &QRadioButton::clicked, this, &ZTakeScreenShotWidget::setFileNameSource);
  lo->addWidget(m_useAutoName);

  if (!m_is2D) {
    hlo = new QHBoxLayout;
    label = m_axis.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_axis.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);

    hlo = new QHBoxLayout;
    label = m_clockwise.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_clockwise.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);

    hlo = new QHBoxLayout;
    label = m_timeInSecond.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_timeInSecond.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);

    hlo = new QHBoxLayout;
    label = m_framePerSecond.createNameLabel();
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setMinimumWidth(125);
    //label->setWordWrap(true);
    hlo->addWidget(label);
    wg = m_framePerSecond.createWidget();
    wg->setMinimumWidth(175);
    wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    hlo->addWidget(wg);
    lo->addLayout(hlo);
  }

  hlo = new QHBoxLayout;
  int left;
  int top;
  int right;
  int bottom;
  hlo->getContentsMargins(&left, &top, &right, &bottom);
  hlo->setContentsMargins(left + 20, top, right, bottom);
  QSettings settings;
  QString folder = settings.value(QString("ScreenShot/exportPath")).toString();
  if (folder.isEmpty())
    folder = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  m_folderWidget = new ZSelectFileWidget(ZSelectFileWidget::FileMode::Directory, "output folder:", QString(),
                                         QBoxLayout::LeftToRight,
                                         folder, this);
  hlo->addWidget(m_folderWidget);
  lo->addLayout(hlo);

  hlo = new QHBoxLayout;
  hlo->setContentsMargins(left + 20, top, right, bottom);
  label = m_namePrefix.createNameLabel();
  label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  label->setMinimumWidth(50);
  //label->setWordWrap(true);
  hlo->addWidget(label);
  wg = m_namePrefix.createWidget();
  wg->setMinimumWidth(175);
  wg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  hlo->addWidget(wg);
  lo->addLayout(hlo);

  m_captureButton = new QPushButton(tr("Capture"), this);
  m_captureButton->setIcon(QIcon(":/icons/screenshot-512.png"));
  m_captureButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lo->addWidget(m_captureButton, 0, Qt::AlignHCenter | Qt::AlignVCenter);

  m_captureSequenceButton = new QPushButton(tr("Capture Sequence"), this);
  m_captureSequenceButton->setIcon(QIcon(":/icons/camcoder_pro-512.png"));
  m_captureSequenceButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  lo->addWidget(m_captureSequenceButton, 0, Qt::AlignHCenter | Qt::AlignVCenter);

  m_useAutoName->click();

  auto widget = new QWidget();
  if (m_group) {
    m_groupBox = new QGroupBox(tr("capture"), this);
    m_groupBox->setLayout(lo);
    hlo = new QHBoxLayout;
    hlo->addWidget(m_groupBox);
    widget->setLayout(hlo);
  } else {
    widget->setLayout(lo);
  }

  setWidgetResizable(true);
  setWidget(widget);
  setFrameShape(QFrame::NoFrame);
}

#include "flyemproofcontrolform.h"

#include <QMenu>
#include <QInputDialog>

#include "ui_flyemproofcontrolform.h"
#include "zdviddialog.h"
#include "zstring.h"
#include "neutubeconfig.h"

FlyEmProofControlForm::FlyEmProofControlForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FlyEmProofControlForm)
{
  ui->setupUi(this);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  connect(ui->segmentCheckBox, SIGNAL(clicked(bool)),
          this, SIGNAL(segmentVisibleChanged(bool)));
  ui->segmentCheckBox->hide();
  connect(ui->mergeSegmentPushButton, SIGNAL(clicked()),
          this, SIGNAL(mergingSelected()));
  connect(ui->dvidPushButton, SIGNAL(clicked()),
          this, SIGNAL(dvidSetTriggered()));
  connect(ui->segmentSizePushButton, SIGNAL(clicked()),
          this, SLOT(setSegmentSize()));
  connect(ui->saveMergePushButton, SIGNAL(clicked()),
          this, SIGNAL(savingMerge()));
  connect(ui->splitPushButton, SIGNAL(clicked()),
          this, SIGNAL(splitTriggered()));
  connect(ui->uploadPushButton, SIGNAL(clicked()),
          this, SIGNAL(committingMerge()));

  ui->segmentSizePushButton->hide();
  ui->segmentSizeDecPushButton->setEnabled(false);


  connect(ui->segmentSizeIncPushButton, SIGNAL(clicked()),
          this, SLOT(incSegmentSize()));
  connect(ui->segmentSizeDecPushButton, SIGNAL(clicked()),
          this, SLOT(decSegmentSize()));

  connect(ui->coarseBodyPushButton, SIGNAL(clicked()),
          this, SIGNAL(coarseBodyViewTriggered()));

  ui->helpWidget->setOpenExternalLinks(true);
  ui->helpWidget->setSource(
        QUrl((GET_DOC_DIR + "/flyem_proofread_help.html").c_str()));


  createMenu();
}

FlyEmProofControlForm::~FlyEmProofControlForm()
{
  delete ui;
}

void FlyEmProofControlForm::createMenu()
{
  m_mainMenu = new QMenu(this);
  ui->menuPushButton->setMenu(m_mainMenu);

  QAction *queryPixelAction = new QAction("Go to Position", this);
  m_mainMenu->addAction(queryPixelAction);
  connect(queryPixelAction, SIGNAL(triggered()), this, SLOT(goToPosition()));

  QAction *queryBodyAction = new QAction("Go to Body", this);
  queryBodyAction->setShortcut(Qt::Key_F1);
  m_mainMenu->addAction(queryBodyAction);
  connect(queryBodyAction, SIGNAL(triggered()), this, SLOT(goToBody()));

  QAction *selectBodyAction = new QAction("Select Body", this);
  m_mainMenu->addAction(selectBodyAction);
  connect(selectBodyAction, SIGNAL(triggered()), this, SLOT(selectBody()));
}

void FlyEmProofControlForm::selectBody()
{
  emit selectingBody();
}

void FlyEmProofControlForm::setSegmentSize()
{
//  emit labelSizeChanged(1024, 1024);
}

void FlyEmProofControlForm::incSegmentSize()
{
  ui->segmentSizeIncPushButton->setEnabled(false);
  ui->segmentSizeDecPushButton->setEnabled(true);
  emit labelSizeChanged(1024, 1024);
}

void FlyEmProofControlForm::decSegmentSize()
{
  ui->segmentSizeIncPushButton->setEnabled(true);
  ui->segmentSizeDecPushButton->setEnabled(false);
  emit labelSizeChanged(512, 512);
}

void FlyEmProofControlForm::goToBody()
{
  emit goingToBody();
}

void FlyEmProofControlForm::goToPosition()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Coordinates:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<int> coords = str.toIntegerArray();
      if (coords.size() == 3) {
        emit zoomingTo(coords[0], coords[1], coords[2]);
      }
    }
  }
}

void FlyEmProofControlForm::setInfo(const QString &info)
{
  ui->dataInfoWidget->setText(info);
}

void FlyEmProofControlForm::setDvidInfo(const ZDvidTarget &target)
{
  setInfo(target.toJsonObject().dumpString(2).c_str());
}

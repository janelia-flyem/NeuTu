#include "flyemprojectdialog.h"

#include <QMenu>

#include "mainwindow.h"
#include "zdviddialog.h"
#include "zdialogfactory.h"
#include "zstackframe.h"
#include "zframefactory.h"
#include "zprogressreporter.h"
#include "zdialogfactory.h"

FlyEmProjectDialog::FlyEmProjectDialog(QWidget *parent) :
  QDialog(parent), m_mainMenu(NULL), m_docTag(NeuTube::Document::NORMAL),
  m_progressReporter(NULL)
{
  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);

  connect(this, SIGNAL(newDocReady(ZStackDocReader*, bool)),
          this, SLOT(consumeNewDoc(ZStackDocReader*, bool)));
}

FlyEmProjectDialog::~FlyEmProjectDialog()
{
  delete m_progressReporter;
}

MainWindow* FlyEmProjectDialog::getMainWindow()
{
  return qobject_cast<MainWindow*>(this->parentWidget());
}

void FlyEmProjectDialog::consumeNewDoc(ZStackDocReader *m_docReader,
                                       bool /*readyForPaint*/)
{
  delete m_docReader;
}

ZStackFrame* FlyEmProjectDialog::newDataFrame(ZStackDocReader &reader)
{
  ZStackFrame *frame = ZFrameFactory::MakeStackFrame(reader, m_docTag);
      //getMainWindow()->createStackFrame(m_docReader, m_docTag);


  getMainWindow()->addStackFrame(frame);
  getMainWindow()->presentStackFrame(frame);

  return frame;
}

void FlyEmProjectDialog::dump(const QString &/*str*/, bool /*appending*/)
{

}

void FlyEmProjectDialog::showInfo(const QString &/*str*/, bool /*appending*/)
{

}

void FlyEmProjectDialog::setDvidDialog(ZDvidDialog *dvidDlg)
{
  m_dvidDlg = dvidDlg;
}

void FlyEmProjectDialog::setDvidTargetD(const ZDvidTarget &dvidTarget)
{
  m_dvidTarget = const_cast<ZDvidTarget&>(dvidTarget);
  dump(QString("Dvid server set to %1").
           arg(m_dvidTarget.getSourceString().c_str()));
}

void FlyEmProjectDialog::advanceProgressSlot(double dp)
{
  if (m_progressReporter != NULL) {
    m_progressReporter->advance(dp);
  }
  //m_progressManager.advanceProgress(dp);
}

void FlyEmProjectDialog::startProgressSlot()
{
  if (m_progressReporter != NULL) {
    m_progressReporter->start();
  }
  //m_progressManager.startProgress();
}

void FlyEmProjectDialog::endProgressSlot()
{
  if (m_progressReporter != NULL) {
    m_progressReporter->end();
  }
  //m_progressManager.endProgress();
}

void FlyEmProjectDialog::setProgressSignalSlot()
{
  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgressSlot(double)));
  connect(this, SIGNAL(progressStarted()), this, SLOT(startProgressSlot()));
  connect(this, SIGNAL(progressEnded()), this, SLOT(endProgressSlot()));
}

void FlyEmProjectDialog::notifyProgressAdvanced(double dp)
{
  emit progressAdvanced(dp);
}

void FlyEmProjectDialog::notifyProgressStarted()
{
  emit progressStarted();
}

void FlyEmProjectDialog::notifyProgressEnded()
{
  emit progressEnded();
}

void FlyEmProjectDialog::setProgressReporter(ZProgressReporter *reporter)
{
  delete m_progressReporter;
  m_progressReporter = reporter;
}

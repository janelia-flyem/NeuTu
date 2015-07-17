#include "zsubtractswcsdialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "zselectfilewidget.h"
#include "zswctree.h"

ZSubtractSWCsDialog::ZSubtractSWCsDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  init();
}

void ZSubtractSWCsDialog::init()
{
  QVBoxLayout *alllayout = new QVBoxLayout;

  m_subtractSWCsWidget = new ZSelectFileWidget(ZSelectFileWidget::OPEN_MULTIPLE_FILES, "Subtract SWCs:",
                                               tr("SWCs (*.swc)"));
  alllayout->addWidget(m_subtractSWCsWidget);

  m_inputSWCWidget = new ZSelectFileWidget(ZSelectFileWidget::OPEN_SINGLE_FILE, "From SWC:",
                                           tr("SWCs (*.swc)"));
  alllayout->addWidget(m_inputSWCWidget);

  alllayout->addSpacing(5);

  m_outputSWCWidget = new ZSelectFileWidget(ZSelectFileWidget::SAVE_FILE, "Output SWC:",
                                            tr("SWC (*.swc)"));
  alllayout->addWidget(m_outputSWCWidget);

  m_runButton = new QPushButton(tr("Run"), this);
  m_exitButton = new QPushButton(tr("Exit"), this);
  m_buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
  m_buttonBox->addButton(m_exitButton, QDialogButtonBox::RejectRole);
  m_buttonBox->addButton(m_runButton, QDialogButtonBox::ActionRole);
  connect(m_exitButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_runButton, SIGNAL(clicked()), this, SLOT(subtractSWCs()));

  alllayout->addWidget(m_buttonBox);

  setLayout(alllayout);
}

void ZSubtractSWCsDialog::subtractSWCs()
{
  if (m_outputSWCWidget->getSelectedSaveFile().isEmpty()) {
    QMessageBox::critical(this, "No output SWC file", "Result SWC file can not be empty.");
    return;
  }

  if (m_inputSWCWidget->getSelectedOpenFile().isEmpty() || m_subtractSWCsWidget->getSelectedMultipleOpenFiles().isEmpty()) {
    QMessageBox::critical(this, "Missing input SWC files", "Input SWC files can not be empty.");
    return;
  }

  ZSwcTree inputSWC;
  if (!inputSWC.load(qPrintable(m_inputSWCWidget->getSelectedOpenFile()))) {
    QMessageBox::critical(this, "Open SWC file error", QString("Can not load SWC file: %1.").arg(m_inputSWCWidget->getSelectedOpenFile()));
    return;
  }
  ZSwcTree subtractSWC;
  QStringList swcList = m_subtractSWCsWidget->getSelectedMultipleOpenFiles();
  if (!subtractSWC.load(qPrintable(swcList.at(0)))) {
    QMessageBox::critical(this, "Open SWC file error", QString("Can not load SWC file: %1.").arg(swcList.at(0)));
    return;
  }
  for (int i=1; i<swcList.size(); ++i) {
    ZSwcTree tmpSWC;
    if (!tmpSWC.load(qPrintable(swcList.at(i)))) {
      QMessageBox::critical(this, "Open SWC file error", QString("Can not load SWC file: %1.").arg(swcList.at(i)));
      return;
    }
    subtractSWC.merge(&tmpSWC, false);
  }
  Swc_Tree_Subtract(inputSWC.data(), subtractSWC.data());
  inputSWC.save(qPrintable(m_outputSWCWidget->getSelectedSaveFile()));

  QMessageBox::information(this, "Done",
                           "Subtract SWCs Finished.");
}


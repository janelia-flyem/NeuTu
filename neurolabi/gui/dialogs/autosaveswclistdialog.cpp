#include "autosaveswclistdialog.h"

#include <iostream>
#include "ui_autosaveswclistdialog.h"
#include "neutubeconfig.h"
#include "zstackframe.h"
#include "tz_error.h"
#include "zwindowfactory.h"

AutosaveSwcListDialog::AutosaveSwcListDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AutosaveSwcListDialog)
{
  ui->setupUi(this);
  ui->listView->setModel(&m_fileList);
  ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(viewSwc(QModelIndex)));
  connect(ui->deleteSelectedPushButton, SIGNAL(clicked()),
          this, SLOT(deleteSelected()));
}

AutosaveSwcListDialog::~AutosaveSwcListDialog()
{
  delete ui;
}

void AutosaveSwcListDialog::updateFile()
{
  m_fileList.loadDir(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE).c_str());
}

QModelIndexList AutosaveSwcListDialog::getSelected() const
{
  QItemSelectionModel *model = ui->listView->selectionModel();

  return model->selectedIndexes();
}

void AutosaveSwcListDialog::deleteSelected()
{
  QModelIndexList indexList = getSelected();
  if (!indexList.isEmpty()) {
    QList<int> rowList;
    foreach (const QModelIndex &index, indexList) {
      rowList.append(index.row());
    }
    qSort(rowList);
    for (int i = rowList.size() - 1; i >= 0; --i) {
      m_fileList.deleteFile(rowList[i]);
    }
  }
}

void AutosaveSwcListDialog::viewSwc(const QModelIndex &index)
{
  QString fileName = m_fileList.data(index).toString();

  if (!fileName.isEmpty()) {
    fileName = "/" + fileName + ".swc";
    fileName =
        NeutubeConfig::getInstance().getPath(NeutubeConfig::AUTO_SAVE).c_str() +
        fileName;

    ZStackDoc *doc = new ZStackDoc;
    ZSwcTree *tree = new ZSwcTree;
    tree->load(fileName.toStdString());
    if (!tree->isEmpty()) {
      doc->addObject(tree);
      ZWindowFactory factory;
      factory.setParentWidget(this->parentWidget());
      Z3DWindow *window = factory.make3DWindow(doc);
      window->show();
      window->raise();
    } else {
      std::cerr << "Cannot open " << fileName.toStdString() << std::endl;
      delete doc;
    }
  }
}

void AutosaveSwcListDialog::on_pushButton_clicked()
{
  ui->listView->setUpdatesEnabled(false);
  m_fileList.deleteAll();
  ui->listView->setUpdatesEnabled(true);
}

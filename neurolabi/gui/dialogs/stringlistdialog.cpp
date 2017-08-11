#include "stringlistdialog.h"
#include "ui_stringlistdialog.h"

StringListDialog::StringListDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::StringListDialog)
{
  ui->setupUi(this);

  m_model = new QStringListModel(this);
  ui->listView->setModel(m_model);
  ui->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(addString()));
  connect(ui->deletePushButton, SIGNAL(clicked()),
          this, SLOT(removeSelectedString()));
  connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
          this, SLOT(removeEmptyString(QModelIndex,QModelIndex)));
}

StringListDialog::~StringListDialog()
{
  delete ui;
}

void StringListDialog::addString(const QString &str)
{
  m_model->insertRow(m_model->rowCount());
  QModelIndex index = m_model->index(m_model->rowCount()-1);
  m_model->setData(index, str);
}

void StringListDialog::addString()
{
  int row = m_model->rowCount();
  m_model->insertRow(row);

  QModelIndex index = m_model->index(row);
  ui->listView->setCurrentIndex(index);
  ui->listView->edit(index);
}

void StringListDialog::removeRowList(const QList<int> &rowList)
{
  QList<int> sortedList = rowList;
  qSort(sortedList);
  for (int i = sortedList.size() - 1; i >= 0; --i) {
    int row = sortedList[i];
    m_model->removeRow(row);
  }
}

void StringListDialog::removeSelectedString()
{
  QItemSelectionModel *model = ui->listView->selectionModel();

  QModelIndexList indexes = model->selectedIndexes();
  QList<int> rowList;
  foreach (const QModelIndex &index, indexes) {
    rowList.append(index.row());
  }
  removeRowList(rowList);
}

void StringListDialog::removeEmptyString()
{
  QStringList stringList = m_model->stringList();

  QList<int> rowList;
  for (int i = 0; i < stringList.size(); ++i) {
    if (stringList[i].isEmpty()) {
      rowList.append(i);
    }
  }
  removeRowList(rowList);
}

void StringListDialog::removeEmptyString(
    const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
  QStringList stringList = m_model->stringList();

  QList<int> rowList;
  for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
    if (stringList[i].isEmpty()) {
      rowList.append(i);
    }
  }
  removeRowList(rowList);
}

void StringListDialog::removeAllString()
{
  m_model->removeRows(0, m_model->rowCount());
}

void StringListDialog::setStringList(const QStringList &list)
{
  m_model->setStringList(list);
}

void StringListDialog::setStringList(const std::vector<std::string> &data)
{
  QStringList strList;
  for (std::vector<std::string>::const_iterator iter = data.begin();
       iter != data.end(); ++iter) {
    strList.append(iter->c_str());
  }
  setStringList(strList);
}

QStringList StringListDialog::getStringList() const
{
  return m_model->stringList();
}

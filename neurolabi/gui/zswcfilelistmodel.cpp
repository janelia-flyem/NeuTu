#include "zswcfilelistmodel.h"

#include <QDir>

int ZSwcFileListModel::maxFileCount = 50;

ZSwcFileListModel::ZSwcFileListModel(QObject *parent) :
  QAbstractListModel(parent)
{
}

QFileInfoList ZSwcFileListModel::LoadDir(
    const QString &dirPath, bool removingExtra)
{
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.swc" << "*.SWC";
  QFileInfoList fileList =
      dir.entryInfoList(filter, QDir::NoFilter, QDir::Time);

  if (fileList.size() > maxFileCount) {
    int fileCount = maxFileCount;
    int originalCount = fileList.size();

    for (int i = fileCount; i < originalCount; ++i) {
      QFileInfo fileInfo = fileList.takeLast();
      if (removingExtra) {
        QFile::remove(fileInfo.absoluteFilePath());
      }
    }
  }

  return fileList;
}

void ZSwcFileListModel::loadDir(const QString &dirPath, bool removingExtra)
{
  m_fileList = LoadDir(dirPath, removingExtra);
#if 0
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.swc" << "*.SWC";
  QFileInfoList fileList =
      dir.entryInfoList(filter, QDir::NoFilter, QDir::Time);

  int fileCount = std::min(maxFileCount, fileList.size());
  m_fileList.clear();
  m_fileList.reserve(fileCount);
  for (int i = 0; i < fileCount; ++i) {
    m_fileList.append(fileList[i]);
  }
  if (removingExtra) {
    for (int i = fileCount; i < fileList.size(); ++i) {
      QFile::remove(fileList[i].absoluteFilePath());
    }
  }
#endif
//  m_fileList = dir.entryInfoList(filter, QDir::NoFilter, QDir::Time);
}

int ZSwcFileListModel::getMaxFileCount() const
{
  return maxFileCount;
}

int ZSwcFileListModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_fileList.size();
}

QVariant ZSwcFileListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= m_fileList.size() || index.row() < 0) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return m_fileList.at(index.row()).completeBaseName();
  }

  return QVariant();
}

bool ZSwcFileListModel::removeRows(int row, int count,
                                   const QModelIndex &parent)
{
  bool removed = false;

  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
      if (row < m_fileList.size()) {
        m_fileList.removeAt(row);
        removed = true;
      }
    }
    endRemoveRows();
  }

  return removed;
}

QString ZSwcFileListModel::getFilePath(int index)
{
  return m_fileList[index].absoluteFilePath();
}

void ZSwcFileListModel::deleteAll()
{
  for (int i = 0; i < rowCount(); ++i) {
    QFile::remove(getFilePath(i));
  }
  removeRows(0, rowCount());
}

void ZSwcFileListModel::deleteFile(int index)
{
  QFile::remove(getFilePath(index));
  removeRows(index, 1);
}

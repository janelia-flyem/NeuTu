#include "zflyembookmarkpresenter.h"
#include <QColor>

ZFlyEmBookmarkPresenter::ZFlyEmBookmarkPresenter(QObject *parent) :
  ZAbstractModelPresenter(parent)
{
  m_fieldList << "X" << "Y" << "Z" << "Body ID" << "User" << "Status"
              << "Time";
}

QVariant ZFlyEmBookmarkPresenter::data(
    const ZFlyEmBookmark &bookmark, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 3:
      return bookmark.getBodyId();
    case 0:
      return bookmark.getLocation().getX();
    case 1:
      return bookmark.getLocation().getY();
    case 2:
      return bookmark.getLocation().getZ();
    case 4:
      return bookmark.getUserName();
    case 5:
      return bookmark.getStatus();
    case 6:
      return bookmark.getTime();
    }
    break;
  case Qt::ToolTipRole:
    switch (index) {
    case 0:
      return QString("Double click to locate the bookmark");
    default:
      break;
    }
    break;
  case Qt::ForegroundRole:
    if (bookmark.isChecked()) {
      return QColor(0, 128, 0);
    }
    break;
  default:
    break;
  }

  return QVariant();
}

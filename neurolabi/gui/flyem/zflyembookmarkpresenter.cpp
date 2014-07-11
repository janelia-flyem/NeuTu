#include "zflyembookmarkpresenter.h"

ZFlyEmBookmarkPresenter::ZFlyEmBookmarkPresenter(QObject *parent) :
  ZAbstractModelPresenter(parent)
{
  m_fieldList << "Body ID" << "X" << "Y" << "Z" << "User" << "Status"
              << "Time";
}

QVariant ZFlyEmBookmarkPresenter::data(
    const ZFlyEmBookmark &bookmark, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      return bookmark.getBodyId();
    case 1:
      return bookmark.getLocation().getX();
    case 2:
      return bookmark.getLocation().getY();
    case 3:
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
  default:
    break;
  }

  return QVariant();
}

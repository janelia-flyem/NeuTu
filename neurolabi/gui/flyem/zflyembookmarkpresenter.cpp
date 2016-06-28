#include "zflyembookmarkpresenter.h"
#include <QColor>

ZFlyEmBookmarkPresenter::ZFlyEmBookmarkPresenter(QObject *parent) :
  ZAbstractModelPresenter(parent)
{
  m_fieldList << "   Type   " << "    Body ID    " << "   Comment   "
              << "      Z      " << "      X      " << "      Y      "
              << "     User     " << "  Status  "
              << "  Time  ";
}

QVariant ZFlyEmBookmarkPresenter::data(
    const ZFlyEmBookmark &bookmark, int index, int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    switch (index) {
    case 0:
      switch (bookmark.getBookmarkType()) {
      case ZFlyEmBookmark::TYPE_FALSE_MERGE:
        return "Split";
      case ZFlyEmBookmark::TYPE_FALSE_SPLIT:
        return "Merge";
      case ZFlyEmBookmark::TYPE_LOCATION:
        return "Other";
      }
      break;
    case 1:
      return QString("%1").arg(bookmark.getBodyId());
    case 2:
      return bookmark.getComment();
    case 3:
      return bookmark.getLocation().getZ();
    case 5:
      return bookmark.getLocation().getY();
    case 4:
      return bookmark.getLocation().getX();
    case 6:
      return bookmark.getUserName();
    case 7:
      return bookmark.getStatus();
    case 8:
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

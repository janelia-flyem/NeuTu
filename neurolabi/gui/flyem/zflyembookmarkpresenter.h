#ifndef ZFLYEMBOOKMARKPRESENTER_H
#define ZFLYEMBOOKMARKPRESENTER_H

#include "zabstractmodelpresenter.h"
#include "zflyembookmark.h"

class ZFlyEmBookmarkPresenter : public ZAbstractModelPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmBookmarkPresenter(QObject *parent = 0);

  virtual QVariant data(
      const ZFlyEmBookmark &bookmark, int index, int role) const;

signals:

public slots:

};

#endif // ZFLYEMBOOKMARKPRESENTER_H

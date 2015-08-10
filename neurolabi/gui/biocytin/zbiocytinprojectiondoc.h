#ifndef ZBIOCYTINPROJECTIONDOC_H
#define ZBIOCYTINPROJECTIONDOC_H

#include "zstackdoc.h"

class ZBiocytinProjectionDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZBiocytinProjectionDoc(QObject *parent = 0);

  void setParentDoc(ZSharedPointer<ZStackDoc> parentDoc);

signals:

public slots:
  bool executeDeleteSwcNodeCommand();

protected:
  void selectSwcNode(const ZRect2d &roi);

private:
  ZSharedPointer<ZStackDoc> m_parentDoc;
};

#endif // ZBIOCYTINPROJECTIONDOC_H

#ifndef ZFLYEMEXTERNALNEURONDOC_H
#define ZFLYEMEXTERNALNEURONDOC_H

#include "zstackdoc.h"

class ZFlyEmExternalNeuronDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmExternalNeuronDoc(QObject *parent = 0);
  virtual ~ZFlyEmExternalNeuronDoc();

  void setDataDoc(ZSharedPointer<ZStackDoc> doc);

signals:

public slots:

private:
  ZSharedPointer<ZStackDoc> m_dataDoc;
};

#endif // ZFLYEMEXTERNALNEURONDOC_H

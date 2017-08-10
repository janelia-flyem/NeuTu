#ifndef ZFLYEMBODY3DDOCKEYPROCESSOR_H
#define ZFLYEMBODY3DDOCKEYPROCESSOR_H

#include <QObject>

#include "zstackdockeyprocessor.h"

//class ZFlyEmBody3dDoc;

class ZFlyEmBody3dDocKeyProcessor : public ZStackDocKeyProcessor
{
  Q_OBJECT
public:
  explicit ZFlyEmBody3dDocKeyProcessor(QObject *parent = 0);


//  ZFlyEmBody3dDoc* getDocument() const;

signals:

public slots:
  bool processKeyEvent(QKeyEvent *event);
  bool processKeyEvent(
      QKeyEvent *event, const ZInteractiveContext &context) override;

};

#endif // ZFLYEMBODY3DDOCKEYPROCESSOR_H

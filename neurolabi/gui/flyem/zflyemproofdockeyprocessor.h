#ifndef ZFLYEMPROOFDOCKEYPROCESSOR_H
#define ZFLYEMPROOFDOCKEYPROCESSOR_H

#include "zstackdockeyprocessor.h"

class ZFlyEmProofDocKeyProcessor : public ZStackDocKeyProcessor
{
  Q_OBJECT
public:
  explicit ZFlyEmProofDocKeyProcessor(QObject *parent = nullptr);

signals:

public slots:
  bool processKeyEvent(
      QKeyEvent *event, const ZInteractiveContext &context) override;
};

#endif // ZFLYEMPROOFDOCKEYPROCESSOR_H

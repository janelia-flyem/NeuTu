#ifndef ZSTACKDOCKEYPROCESSOR_H
#define ZSTACKDOCKEYPROCESSOR_H

#include <QObject>

#include "zstackoperator.h"

class QKeyEvent;
class ZInteractiveContext;

class ZStackDocKeyProcessor : public QObject
{
  Q_OBJECT

public:
  ZStackDocKeyProcessor(QObject *parent = 0);

  template<typename T>
  T* getDocument() const {
    return qobject_cast<T*>(parent());
  }

  const ZStackOperator& getOperator() const {
    return m_operator;
  }

public slots:
  virtual bool processKeyEvent(QKeyEvent *event);
  virtual bool processKeyEvent(
      QKeyEvent *event, const ZInteractiveContext &context);

protected:
  ZStackOperator m_operator;
};


#endif // ZSTACKDOCKEYPROCESSOR_H

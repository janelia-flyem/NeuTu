#ifndef ZSTACKDOCKEYPROCESSOR_H
#define ZSTACKDOCKEYPROCESSOR_H

#include <QObject>

class QKeyEvent;

class ZStackDocKeyProcessor : public QObject
{
  Q_OBJECT

public:
  ZStackDocKeyProcessor(QObject *parent = 0);

  template<typename T>
  T* getDocument() const {
    return qobject_cast<T*>(parent());
  }

public slots:
  virtual bool processKeyEvent(QKeyEvent *event);
};


#endif // ZSTACKDOCKEYPROCESSOR_H

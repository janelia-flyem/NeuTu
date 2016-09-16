#ifndef ZRGB2GRAYMODULE_H
#define ZRGB2GRAYMODULE_H

#include "zsandboxmodule.h"

class ZRgb2GrayModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZRgb2GrayModule(QObject *parent = 0);

  QAction* getAction() const;

signals:

public slots:

private slots:
  void execute();

private:
  void init();


private:
  QAction *m_action;
};

#endif // ZRGB2GRAYMODULE_H

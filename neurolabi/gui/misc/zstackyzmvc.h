#ifndef ZSTACKYZMVC_H
#define ZSTACKYZMVC_H

#include "zstackmvc.h"

class ZStackYZMvc : public ZStackMvc
{
  Q_OBJECT
public:
  explicit ZStackYZMvc(QWidget *parent = 0);

  static ZStackYZMvc* Make(QWidget *parent, ztr1::shared_ptr<ZStackDoc> doc);

signals:

public slots:

protected:
  virtual void createView();
};

#endif // ZSTACKYZMVC_H

#ifndef ZTILEDSTACKFRAME_H
#define ZTILEDSTACKFRAME_H

#include "zstackframe.h"

class ZTileManager;

class ZTiledStackFrame : public ZStackFrame
{
  Q_OBJECT
public:
  explicit ZTiledStackFrame(QWidget *parent = 0, bool preparingModel = true);

  inline ZTileManager* getTileManager() {
    return m_tileManager;
  }

  bool importTiles(const QString &path);

signals:

public slots:

private:
  ZTileManager *m_tileManager;
};

#endif // ZTILEDSTACKFRAME_H

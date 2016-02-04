#ifndef ZTILEDSTACKFRAME_H
#define ZTILEDSTACKFRAME_H

#include "zstackframe.h"

class ZTileManager;

class ZTiledStackFrame : public ZStackFrame
{
  Q_OBJECT
protected:
  explicit ZTiledStackFrame(QWidget *parent = 0);

public:
  static ZTiledStackFrame *Make(QMdiArea *parent);
  static ZTiledStackFrame* Make(QMdiArea *parent, ZSharedPointer<ZStackDoc> doc);

  inline ZTileManager* getTileManager() {
    return m_tileManager;
  }

  bool importTiles(const QString &path);

  void updateStackBoundBox();

signals:

public slots:

private:
  ZTileManager *m_tileManager;
};

#endif // ZTILEDSTACKFRAME_H

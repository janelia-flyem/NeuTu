#ifndef ZTILEMANAGERVIEW_H
#define ZTILEMANAGERVIEW_H

#include <QGraphicsView>
#include <tilemanager.h>

class ZTileManagerView : public QGraphicsView
{
  Q_OBJECT
public:
  explicit ZTileManagerView(QWidget *parent = 0);

  void paintEvent(QPaintEvent *event);
  inline TileManager *getParentWindow() { return m_parentWindow; }
  inline void setParentWindow(TileManager * p) { m_parentWindow = p;}
  inline void setSWCVisibility(bool vis) {swcVisible = vis; }

signals:

public slots:
//  void slotTest();

private:
  TileManager *m_parentWindow;
  bool swcVisible;
};

#endif // ZTILEMANAGERVIEW_H

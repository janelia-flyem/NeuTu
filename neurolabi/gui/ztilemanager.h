#ifndef ZTILEMANAGER_H
#define ZTILEMANAGER_H

#include <QGraphicsScene>
#include <QVector>
#include "zprogressable.h"
#include "ztilemanagerview.h"
#include <QList>

class ZStackFrame;
class ZTileGraphicsItem;

class ZTileManager : public QGraphicsScene, public ZProgressable
{
  Q_OBJECT
public:
  explicit ZTileManager(QObject *parent = 0);
  ~ZTileManager();

  /*!
   * \brief Load tiles from a json file
   *
   * \return true iff the tile is loaded successfully.
   */
  bool importJsonFile(const QString &filePath);

  ZStackFrame *getParentFrame() const;

  /*!
   * \brief getFirstTile
   * \return
   */
  ZTileGraphicsItem* getFirstTile();

  //void preselectItem(ZTileGraphicsItem *item);
  void selectItem(ZTileGraphicsItem *item);
  void updateTileStack();
//  void setScaleFactor(float sf) {scaleFactor = sf;}
  void setParentView(ZTileManagerView *p) {m_view = p; }
  ZTileManagerView *getParentView() {return m_view; }
  ZTileGraphicsItem* getSelectedTileItem(){return m_selectedTileItem; }

signals:
  void loadingTile();

public slots:

protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent);
  void initDecoration();
  //void clearPreselected();
  void plotItemBoundary(ZTileGraphicsItem *item, QColor boundaryColor);

private:
  ZTileGraphicsItem *m_selectedTileItem;
  //ZTileGraphicsItem *m_preselected;
  QGraphicsRectItem *m_highlightRec;
//  float scaleFactor;
  ZTileManagerView *m_view;

  ZResolution m_resolution;

  //static const QColor m_preselectionColor;
  static const QColor m_selectionColor;
  //QGraphicsRectItem *m_selectDecoration; //always owned by the scene
};

#endif // ZTILEMANAGER_H

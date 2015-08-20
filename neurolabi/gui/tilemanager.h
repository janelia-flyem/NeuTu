#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "zstackdoc.h"
#include "zsharedpointer.h"

class ZTileManager;
class ZQtBarProgressReporter;
class QKeyEvent;

namespace Ui {
class TileManager;
}

class TileManager : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit TileManager(QWidget *parent = 0);
    ~TileManager();

    void setTileManager(ZTileManager *manager);
    void setDocument(ZSharedPointer<ZStackDoc> p_doc);
    inline ZSharedPointer<ZStackDoc> getDocument() const { return m_doc; }
    inline float getScaleFactor() {return scaleFactor;}

private slots:
    void ShowContextMenu(const QPoint& pos);
    void on_actionShowSWC_triggered();
    void on_actionTurnOffSWC_triggered();
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::TileManager *ui;
    ZQtBarProgressReporter *m_progressReporter;
    ZSharedPointer<ZStackDoc> m_doc;
    float scaleFactor;
};

#endif // TILEMANAGER_H


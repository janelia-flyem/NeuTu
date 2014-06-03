#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <tr1/memory>
#include "zstackdoc.h"
#include <QKeyEvent>

class ZTileManager;
class ZQtBarProgressReporter;

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
    void setDocument(std::tr1::shared_ptr<ZStackDoc> p_doc);
    inline ZStackDoc* getDocument() const { return m_doc.get(); }
    inline float getScaleFactor() {return scaleFactor;}

private slots:
    void ShowContextMenu(const QPoint& pos);
    void on_actionShowSWC_triggered();
    void on_actionTurnOffSWC_triggered();
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::TileManager *ui;
    ZQtBarProgressReporter *m_progressReporter;
    std::tr1::shared_ptr<ZStackDoc> m_doc;
    float scaleFactor;
};

#endif // TILEMANAGER_H


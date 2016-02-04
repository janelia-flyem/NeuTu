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
//    inline float getScaleFactor() {return m_scaleFactor;}

public slots:
    void closeProject();

private slots:
    void ShowContextMenu(const QPoint& pos);
    void on_actionShowSWC_triggered();
    void on_actionTurnOffSWC_triggered();
    void keyPressEvent(QKeyEvent *event);
    void showSwc(bool on);

private:
    void init();
    void createMenu();
    void connectSignalSlot();
    void updateView();
    void zoom(int ds);

private:
    Ui::TileManager *ui;
    ZQtBarProgressReporter *m_progressReporter;
    ZSharedPointer<ZStackDoc> m_doc;
    int m_scaleFactor;
    QMenu *m_contextMenu;
    QAction *m_showSwcAction;
};

#endif // TILEMANAGER_H


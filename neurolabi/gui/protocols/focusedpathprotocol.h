#ifndef FOCUSEDPATHPROTOCOL_H
#define FOCUSEDPATHPROTOCOL_H

#include <QtGui>

#include "dvid/zdvidreader.h"
#include "zjsonobject.h"

#include "focusedpath.h"
#include "focusededge.h"
#include "protocoldialog.h"


namespace Ui {
class FocusedPathProtocol;
}

class FocusedPathProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit FocusedPathProtocol(QWidget *parent = 0, std::string variation = VARIATION_BODY);
    ~FocusedPathProtocol();
    bool initialize();
    static const std::string VARIATION_BODY;
    static const std::string VARIATION_BOOKMARK;
    void setDvidTarget(ZDvidTarget target);

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);
    void bodyListLoaded();
    void currentBodyPathsLoaded();

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();
    void onBodyListsLoaded();
    void onCurrentBodyPathsLoaded();    
    void onEdgeSelectionChanged(QItemSelection oldItem, QItemSelection newItem);

private:
    static const std::string KEY_VERSION;
    static const int m_fileVersion;
    // load/save protocol keys
    static const std::string KEY_VARIATION;
    static const std::string KEY_BODYID;
    static const std::string KEY_EDGE_INSTANCE;
    static const std::string KEY_PATH_INSTANCE;
    static const std::string KEY_POINT_INSTANCE;
    enum EdgeTableColumns {
        BODYID1_COLUMN,
        CONNECTION_COLUMN,
        BODYID2_COLUMN
        };

    static const QColor COLOR_BODY1;
    static const QColor COLOR_BODY2;
    static const QColor COLOR_EDGE1;
    static const QColor COLOR_EDGE2;
    static const QColor COLOR_PATH;
    static const QColor COLOR_OTHER;
    static const QColor COLOR_DEFAULT;

public:
    // keys for DVID stuff
    static const std::string KEY_ASSIGNMENT_BODIES;
    static const std::string KEY_ASSIGNMENT_EDGE_INSTANCE;
    static const std::string KEY_ASSIGNMENT_PATH_INSTANCE;
    static const std::string KEY_ASSIGNMENT_POINT_INSTANCE;
    static const std::string TAG_PATH;
    static const std::string TAG_EDGE;
    static const std::string PROPERTY_EDGES;
    static const std::string PROPERTY_PATHS;

private:
    Ui::FocusedPathProtocol *ui;
    QWidget * m_parent;
    ZDvidReader m_reader;
    std::string m_variation;
    std::string m_edgeDataInstance;
    std::string m_pathDataInstance;
    std::string m_pointDataInstance;
    QList<uint64_t> m_bodies;
    uint64_t m_currentBody;
    QList<FocusedPath> m_currentBodyPaths;
    QMap<ZIntPoint, uint64_t> m_currentPathBodyIDs;
    FocusedPath m_currentPath;
    QStandardItemModel * m_edgeModel;
    ZFlyEmSequencerColorScheme m_colorScheme;
    void saveState();
    void variationError(std::string variation);
    void loadBodiesFromBookmarks();
    void loadCurrentBodyPaths(uint64_t bodyID);
    FocusedPath findNextPath();
    void displayCurrentPath();
    void deletePath(FocusedPath path);
    void updateConnectionLabel();
    void updateProgressLabel();
    void updateColorMap();
    void gotoEdgePoint(FocusedEdge edge);
    FocusedEdge getSelectedEdge();
    void printEdge(FocusedEdge edge);
    void printPath(FocusedPath path);
    std::string getPropertyKey(std::string prefix, ZIntPoint point);
};

#endif // FOCUSEDPATHPROTOCOL_H

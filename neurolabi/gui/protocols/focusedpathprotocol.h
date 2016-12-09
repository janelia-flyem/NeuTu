#ifndef FOCUSEDPATHPROTOCOL_H
#define FOCUSEDPATHPROTOCOL_H

#include <QtGui>

#include "dvid/zdvidreader.h"
#include "zjsonobject.h"

#include "focusedpath.h"
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

private:
    static const std::string KEY_VERSION;
    static const int m_fileVersion;
    // load/save protocol keys
    static const std::string KEY_VARIATION;
    static const std::string KEY_BODYID;
    static const std::string KEY_EDGE_INSTANCE;
    // keys for DVID stuff
    static const std::string KEY_ASSIGNMENT_BODIES;
    static const std::string KEY_ASSIGNMENT_INSTANCE;
    static const std::string TAG_PATH;
    static const std::string TAG_EDGE;


    Ui::FocusedPathProtocol *ui;
    QWidget * m_parent;
    ZDvidReader m_reader;
    std::string m_variation;
    std::string m_edgeDataInstance;
    QList<uint64_t> m_bodies;
    uint64_t m_currentBody;
    QList<FocusedPath> m_currentBodyPaths;
    QStandardItemModel * m_edgeModel;
    void saveState();
    void variationError(std::string variation);
    void loadBodiesFromBookmarks();
    void loadCurrentBodyPaths(uint64_t bodyID);
};

#endif // FOCUSEDPATHPROTOCOL_H

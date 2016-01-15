#ifndef FLYEMBODYINFODIALOG_H
#define FLYEMBODYINFODIALOG_H

#include <QDialog>
#include <QtGui>

#include "dvid/zdvidtarget.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "flyem/zflyemsequencercolorscheme.h"

namespace Ui {
class FlyEmBodyInfoDialog;
}

class FlyEmBodyInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlyEmBodyInfoDialog(QWidget *parent = 0);
    ~FlyEmBodyInfoDialog();

public slots:
  void dvidTargetChanged(ZDvidTarget target);

signals:
  void bodyActivated(uint64_t bodyId);
  void bodiesActivated(QList<uint64_t> bodyIdList);
  void addBodyActivated(uint64_t bodyId);
  void dataChanged(ZJsonValue object);
  void jsonLoadBookmarksError(QString message);
  void jsonLoadColorMapError(QString message);
  void loadCompleted();
  void colorMapChanged(ZFlyEmSequencerColorScheme scheme);
  void colorMapLoaded(ZJsonValue colors);
  void inputBodiesLoaded(ZJsonValue bodies);

private slots:
    void onCloseButton();
    void onRefreshButton();
    void onDoubleClickBodyTable(QModelIndex modelIndex);
    void activateBody(QModelIndex modelIndex);
    void updateModel(ZJsonValue object);
    void onjsonLoadBookmarksError(QString message);
    void onjsonLoadColorMapError(QString message);
    void updateStatusLabel();
    void updateStatusAfterLoading();
    void updateBodyFilterAfterLoading();
    void bodyFilterUpdated(QString filterText);
    void applicationQuitting();    
    void onSaveColorFilter();
    void onFilterTableDoubleClicked(const QModelIndex &index);
    void moveToBodyList();
    void onDeleteButton();
    void onExportBodies();
    void onSaveColorMap();
    void onLoadColorMap();
    void onColorMapLoaded(ZJsonValue colors);
    void updateColorScheme();
    void onGotoBodies();    
    void onInputBodiesLoaded(ZJsonValue bodies);

private:
    enum Tabs {
        COLORS_TAB,
        CONNECTIONS_TAB
        };
    enum BodyTableColumns {
        BODY_ID_COLUMN,
        BODY_NAME_COLUMN,
        BODY_NPRE_COLUMN,
        BODY_NPOST_COLUMN,
        BODY_STATUS_COLUMN
    };
    enum FilterTableColumns {
        FILTER_NAME_COLUMN,
        FILTER_COLOR_COLUMN
    };
    enum IOBodyTableColumns {
        IOBODY_ID_COLUMN,
        IOBODY_NAME_COLUMN,
        IOBODY_NUMBER_COLUMN
    };
    Ui::FlyEmBodyInfoDialog *ui;
    QStandardItemModel* m_bodyModel;
    QStandardItemModel* m_filterModel;
    QStandardItemModel* m_inputBodyModel;
    QSortFilterProxyModel* m_bodyProxy;
    QSortFilterProxyModel* m_filterProxy;
    QSortFilterProxyModel* m_inputBodyProxy;
    QSortFilterProxyModel* m_schemeBuilderProxy;
    ZFlyEmSequencerColorScheme m_colorScheme;
    qlonglong m_totalPre;
    qlonglong m_totalPost;
    bool m_quitting;
    ZDvidTarget m_currentDvidTarget;
    void setBodyHeaders(QStandardItemModel*);
    void setFilterHeaders(QStandardItemModel*);
    bool isValidBookmarkFile(ZJsonObject object);
    bool dvidBookmarksPresent(ZDvidTarget target);
    bool bodies3Present(ZDvidTarget target);
    void importBookmarksDvid(ZDvidTarget target);
    void importBodiesDvid(ZDvidTarget target);
    void setStatusLabel(QString label);
    void clearStatusLabel();
    void init();
    void updateColorFilter(QString filter, QString oldFilter = "");
    void exportBodies(QString filename);
    void saveColorMapDisk(QString filename);
    ZJsonArray getColorMapAsJson(ZJsonArray colors);
    bool isValidColorMap(ZJsonValue colors);
    void setFilterTableModelColor(QColor color, int modelRow);
    void gotoPrePost(QModelIndex modelIndex);
    void updateBodyConnectionLabel(uint64_t bodyID, QString bodyName);
    void setInputBodyHeaders(QStandardItemModel *model);
    void retrieveIOBodiesDvid(ZDvidTarget target);
};

#endif // FLYEMBODYINFODIALOG_H

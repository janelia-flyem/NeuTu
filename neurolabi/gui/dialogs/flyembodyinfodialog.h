#ifndef FLYEMBODYINFODIALOG_H
#define FLYEMBODYINFODIALOG_H

#include <QDialog>
#include <QtGui>

#include "dvid/zdvidtarget.h"
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
  void dataChanged(ZJsonValue object);
  void jsonLoadError(QString message);
  void loadCompleted();
  void colorMapChanged(ZFlyEmSequencerColorScheme scheme);

private slots:
    void onCloseButton();
    void onRefreshButton();
    void activateBody(QModelIndex modelIndex);
    void updateModel(ZJsonValue object);
    void onJsonLoadError(QString message);
    void updateStatusLabel();
    void updateStatusAfterLoading();
    void updateBodyFilterAfterLoading();
    void bodyFilterUpdated(QString filterText);
    void applicationQuitting();    
    void onSaveColorFilter();
    void onFilterTableDoubleClicked(const QModelIndex &index);
    void moveToBodyList();
    void onDeleteButton();

private:
    Ui::FlyEmBodyInfoDialog *ui;
    QStandardItemModel* m_bodyModel;
    QStandardItemModel* m_filterModel;
    QSortFilterProxyModel* m_bodyProxy;
    QSortFilterProxyModel* m_filterProxy;
    QSortFilterProxyModel* m_schemeBuilderProxy;
    ZFlyEmSequencerColorScheme m_colorScheme;
    qlonglong m_totalPre;
    qlonglong m_totalPost;
    bool m_quitting;
    ZDvidTarget m_currentDvidTarget;
    QStandardItemModel* createModel(QObject*);
    QStandardItemModel* createFilterModel(QObject*);
    void setHeaders(QStandardItemModel*);
    bool isValidBookmarkFile(ZJsonObject object);
    bool dvidBookmarksPresent(ZDvidTarget target);
    bool bodies3Present(ZDvidTarget target);
    void importBookmarksDvid(ZDvidTarget target);
    void importBodiesDvid(ZDvidTarget target);
    void setStatusLabel(QString label);
    void clearStatusLabel();
    void init();
    void updateColorFilter(QString filter, QString oldFilter = "");
    void updateColorScheme();
};

#endif // FLYEMBODYINFODIALOG_H

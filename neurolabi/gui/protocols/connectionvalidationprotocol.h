#ifndef CONNECTIONVALIDATIONPROTOCOL_H
#define CONNECTIONVALIDATIONPROTOCOL_H

#include <QDialog>
#include <QMap>
#include <QStandardItemModel>
#include <QModelIndex>

#include "protocoldialog.h"

#include "zjsonobject.h"
#include "geometry/zintpoint.h"

namespace Ui {
class ConnectionValidationProtocol;
}

class ConnectionValidationProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit ConnectionValidationProtocol(QWidget *parent = 0);
    ~ConnectionValidationProtocol();

    bool initialize();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();

    void onFirstButton();
    void onNextButton();
    void onMarkAndNextButton();
    void onGotoButton();

    void onReviewedChanged();
    void onTbarGoodChanged();
    void onTbarSegGoodChanged();
    void onPSDGoodCanged();
    void onPSDSegGoodChanged();
    void onNotSureChanged();

    void onSetComment();

    void onClickedTable(QModelIndex index);

private:
    // Task attribute control (prefixed with ac)
    bool acReviewed() const;
    void acSetReviewed(bool on);
    bool acTbarGood() const;
    void acSetTbarGood(bool on);
    bool acPsdGood() const;
    void acSetPsdGood(bool on);
    bool acTbarSegGood() const;
    void acSetTbarSegGood(bool on);
    bool acPsdSegGood() const;
    void acSetPsdSegGood(bool on);
    bool acNotSure() const;
    void acSetNotSure(bool on);
    QString acComment() const;
    void acSetComment(const QString &comment);

    void updateTimestamp();

private:
    static const std::string KEY_VERSION;
    static const int fileVersion;

    struct PointData {
        bool reviewed = false;
        bool tbarGood = false;
        bool tbarSegGood = false;
        bool psdGood = false;
        bool psdSegGood = false;
        bool notSure = false;
        int64_t timestamp = 0;
        QString comment;
    };

    enum SitesTableColumns {
        POINT_COLUMN,
        REVIEWED_COLUMN,
        TBAR_GOOD_COLUMN,
        PSD_GOOD_COLUMN,
        TBAR_SEG_GOOD_COLUMN,
        PSD_SEG_GOOD_COLUMN,
        NOT_SURE_COLUMN,
        HAS_COMMENT_COLUMN
    };

    Ui::ConnectionValidationProtocol *ui;
    QList<ZIntPoint> m_points;
    QMap<ZIntPoint, PointData> m_pointData;
    QStandardItemModel * m_sitesModel;
    int m_currentIndex;
    QString m_assignmentID;


    void applyChange();
    void saveState();
    void showMessage(QString title, QString message);
    void showError(QString title, QString message);
    void showCompleteMessage();
    void loadPoints(QJsonArray array);
    void setSitesHeaders(QStandardItemModel *model);
    void clearSitesTable();

    void setCurrentPoint(int index);
    void selectCurrentRow();
    void gotoCurrentPoint();
    void setSelectGotoCurrentPoint(int index);

    int findFirstUnreviewed();
    int findNextUnreviewed();
    void setCurrentReviewed();
    void presentTask(int index);

    void updateLabels();
    void updateCurrentLabel();
    void updateProgressLabel();
    void updateCheckBoxes();
    void updateComment();
    void updateTable();

};

#endif // CONNECTIONVALIDATIONPROTOCOL_H


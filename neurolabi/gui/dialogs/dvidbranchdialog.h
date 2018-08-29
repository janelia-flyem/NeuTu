#ifndef DVIDBRANCHDIALOG_H
#define DVIDBRANCHDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QModelIndex>
#include <QNetworkReply>
#include <QStringListModel>

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "zdvidtargetproviderdialog.h"

namespace Ui {
class DvidBranchDialog;
}

class DvidBranchDialog : public ZDvidTargetProviderDialog
{
    Q_OBJECT

public:
    explicit DvidBranchDialog(QWidget *parent = 0);
    ~DvidBranchDialog();

    ZDvidTarget& getDvidTarget();
    const ZDvidTarget& getDvidTarget(const std::string &name) const;

private slots:
    void onRepoClicked(QModelIndex modelIndex);
    void onBranchClicked(QModelIndex modelIndex);
    void toggleDetailsPanel();
    void launchOldDialog();    
    void finishLoadingDatasets(QJsonObject jsonData);
    void finishLoadingDatasetsFromConfigServer(QNetworkReply::NetworkError error = QNetworkReply::NoError);

signals:
    void datasetsFinishedLoading(QJsonObject jsonData);

private:
    static const QString KEY_DATASETS;
    static const QString KEY_VERSION;
    static const QString KEY_CONFIG;
    static const int SUPPORTED_VERSION;
    static const QString URL_DATASETS;
    static const QString KEY_NAME;
    static const QString KEY_SERVER;
    static const QString KEY_PORT;
    static const QString KEY_UUID;
    static const QString KEY_DESCRIPTION;
    static const QString KEY_DAG;
    static const QString KEY_NODES;
    static const QString KEY_NOTE;

    static const QString INSTANCE_BRANCHES;
    static const QString KEY_MASTER;

    static const int DEFAULT_PORT;

    static const QString DEFAULT_MASTER_NAME;
    static const QString MESSAGE_LOADING;
    static const QString MESSAGE_ERROR;

    Ui::DvidBranchDialog *ui;
    ZDvidReader m_reader;
    ZDvidTarget m_dvidTarget;
    QStringListModel * m_repoModel;
    QString m_repoName;
    QStringListModel * m_branchModel;
    QString m_branchName;
    QMap<QString, QJsonObject> m_repoMap;
    QMap<QString, QJsonObject> m_branchMap;
    bool m_detailsVisible;
    QNetworkAccessManager * m_networkManager;
    QNetworkReply * m_datasetReply = nullptr;

    void loadDatasets();
    void loadDatasetsFromFile();
    void loadBranches(QString repoName);
    QStringList findBranches(QJsonObject nodeJson);
    void showError(QString title, QString message);
    void loadNode(QString branchName);
    void clearNode();
    QString findMasterName(QString prefix, QStringList names);
    void showDetailsPanel();
    void hideDetailsPanel();
    void displayDatasetError(QString errorMessage);
    void loadDatasetsFromConfigServer();
    void showEvent(QShowEvent *event);
};

#endif // DVIDBRANCHDIALOG_H

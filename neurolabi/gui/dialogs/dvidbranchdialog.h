#ifndef DVIDBRANCHDIALOG_H
#define DVIDBRANCHDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QModelIndex>
#include <QStringListModel>

#include "dvid/zdvidreader.h"

namespace Ui {
class DvidBranchDialog;
}

class DvidBranchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DvidBranchDialog(QWidget *parent = 0);
    ~DvidBranchDialog();

private slots:
    void onRepoClicked(QModelIndex modelIndex);
    void onBranchClicked(QModelIndex modelIndex);
    void toggleDetailsPanel();

private:
    static const QString KEY_REPOS;
    static const QString KEY_NAME;
    static const QString KEY_SERVER;
    static const QString KEY_PORT;
    static const QString KEY_UUID;
    static const QString KEY_DESCRIPTION;
    static const QString KEY_DAG;
    static const QString KEY_NODES;
    static const QString KEY_NOTE;

    static const int DEFAULT_PORT;

    static const QString DEFAULT_MASTER_NAME;
    static const QString MESSAGE_LOADING;

    Ui::DvidBranchDialog *ui;
    ZDvidReader m_reader;
    QStringListModel * m_repoModel;
    QString m_repoName;
    QStringListModel * m_branchModel;
    QString m_branchName;
    QMap<QString, QJsonObject> m_repoMap;
    QMap<QString, QJsonObject> m_branchMap;
    bool m_detailsVisible;

    void loadDatasets();
    QJsonObject loadDatasetsFromFile();
    void loadBranches(QString repoName);
    QStringList findBranches(QJsonObject nodeJson);
    void showError(QString title, QString message);
    void loadNode(QString branchName);
    void clearNode();
    QString findMasterName(QString prefix, QStringList names);
    void showDetailsPanel();
    void hideDetailsPanel();
};

#endif // DVIDBRANCHDIALOG_H

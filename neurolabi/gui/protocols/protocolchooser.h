#ifndef PROTOCOLCHOOSER_H
#define PROTOCOLCHOOSER_H

#include <QDialog>
#include <QModelIndex>
#include <QStringListModel>

namespace Ui {
class ProtocolChooser;
}

class ProtocolChooser : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolChooser(QWidget *parent = 0);
    ~ProtocolChooser();

public slots:
    void displaySavedProtocolKeys(QStringList keyList);

signals:
    void requestStartProtocol(QString protocolName);
    void requestLoadProtocolKey(QString protocolKey);

private slots:
    void onLoadButton();
    void onStartButton();
    void onDoubleClickedStartProtocol(QModelIndex modelIndex);
    void onDoubleClickedLoadProtocol(QModelIndex modelIndex);

private:
    Ui::ProtocolChooser *ui;
    QStringListModel * m_savedProtocolListModel;

    void setupNewProtocolList();
    void setupSavedProtocolList();
};

#endif // PROTOCOLCHOOSER_H

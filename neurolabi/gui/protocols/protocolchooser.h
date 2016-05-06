#ifndef PROTOCOLCHOOSER_H
#define PROTOCOLCHOOSER_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
class ProtocolChooser;
}

class ProtocolChooser : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolChooser(QWidget *parent = 0);
    ~ProtocolChooser();

signals:
    void requestStartProtocol(QString protocolName);

private slots:
    void onLoadButton();
    void onStartButton();
    void onDoubleClickedStartProtocol(QModelIndex modelIndex);

private:
    Ui::ProtocolChooser *ui;

    void setupNewProtocolList();
};

#endif // PROTOCOLCHOOSER_H

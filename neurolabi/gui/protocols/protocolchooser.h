#ifndef PROTOCOLCHOOSER_H
#define PROTOCOLCHOOSER_H

#include <QDialog>

namespace Ui {
class ProtocolChooser;
}

class ProtocolChooser : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolChooser(QWidget *parent = 0);
    ~ProtocolChooser();

private:
    Ui::ProtocolChooser *ui;
};

#endif // PROTOCOLCHOOSER_H

#include "protocolchooser.h"
#include "ui_protocolchooser.h"

ProtocolChooser::ProtocolChooser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolChooser)
{
    ui->setupUi(this);
}

ProtocolChooser::~ProtocolChooser()
{
    delete ui;
}

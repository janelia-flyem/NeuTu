#ifndef PROTOCOLDIALOG_H
#define PROTOCOLDIALOG_H

#include <QDialog>

namespace Ui {
class ProtocolDialog;
}

class ProtocolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolDialog(QWidget *parent = 0);
    ~ProtocolDialog();    
    void setNThings(int nThings);
    int getNThings();
    virtual bool initialize();

private:
    Ui::ProtocolDialog *ui;
    int m_nThings;
};

#endif // PROTOCOLDIALOG_H

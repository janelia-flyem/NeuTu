#ifndef ORPHANLINKINPUTDIALOG_H
#define ORPHANLINKINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class OrphanLinkInputDialog;
}

class OrphanLinkInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrphanLinkInputDialog(QStringList projectList, QWidget *parent = 0);
    ~OrphanLinkInputDialog();

    QString getProject();

private slots:
    void onGetButton();

private:
    Ui::OrphanLinkInputDialog *ui;

    QString m_project;
};

#endif // ORPHANLINKINPUTDIALOG_H

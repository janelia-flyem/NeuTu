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

private slots:
    void onGetButton();

private:
    Ui::OrphanLinkInputDialog *ui;

    QString m_project;

    QString getProject();
};

#endif // ORPHANLINKINPUTDIALOG_H

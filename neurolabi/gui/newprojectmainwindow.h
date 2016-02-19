#ifndef NEWPROJECTMAINWINDOW_H
#define NEWPROJECTMAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
class NewProjectMainWindow;
}

class NewProjectMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit NewProjectMainWindow(QWidget *parent = 0);
    ~NewProjectMainWindow();

private:
    Ui::NewProjectMainWindow *ui;

private slots:
    void openTifStacks();
    void filenameCommonEntered();
    void processStacks();

private:
    QDir filenameDir;
    QString filenameCommon;
    QStringList stackFilenames;

private:
    void displayStackInfo();
};

#endif // NEWPROJECTMAINWINDOW_H

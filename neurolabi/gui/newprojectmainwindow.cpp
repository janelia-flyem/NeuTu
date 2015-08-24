#include "newprojectmainwindow.h"
#include "ui_newprojectmainwindow.h"
#include <QFileDialog>
#include "zstack.hxx"
#include "zimage.h"
#include "biocytin/zstackprojector.h"

NewProjectMainWindow::NewProjectMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NewProjectMainWindow)
{
    ui->setupUi(this);
    setWindowTitle("New Project: Preprocess Tif Stacks");
    ui->filenameCommon->setDisabled(true);
    ui->messages->setDisabled(true);
    ui->processButton->setVisible(false);
    ui->progressBar->setVisible(false);
    connect(ui->openStacksButton,SIGNAL(clicked()),this,SLOT(openTifStacks()));
    connect(ui->filenameCommon,SIGNAL(returnPressed()),this,SLOT(filenameCommonEntered()));
    connect(ui->processButton,SIGNAL(clicked()),this,SLOT(processStacks()));
}

NewProjectMainWindow::~NewProjectMainWindow()
{
    delete ui;
}

void NewProjectMainWindow::openTifStacks()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Select one tif stack",NULL,"*[0-9]?.tif");
    if (!fileName.isEmpty()) {
        QFileInfo fInfo(fileName);
        filenameCommon = fInfo.fileName().remove(QRegExp("([0-9]+).tif"));
        filenameDir = fInfo.absoluteDir();
        displayStackInfo();
    }
}

void NewProjectMainWindow::filenameCommonEntered()
{
    filenameCommon = ui->filenameCommon->text();
    filenameCommon.remove("\n");
    stackFilenames.clear();
    displayStackInfo();
}

void NewProjectMainWindow::displayStackInfo()
{
    ui->filenameCommon->setText(filenameCommon);
    ui->messages->append(tr("Tif stack directory: ")+filenameDir.absolutePath());
    ui->messages->append(tr("Tif filename common: ")+filenameCommon);
    QStringList filter;
    filter <<filenameCommon.append("([0-9]).tif");
    //get all stack files
    filenameDir.setNameFilters(filter);
    stackFilenames = filenameDir.entryList();
    ui->messages->append(tr("Found ")+QString::number(stackFilenames.size())+tr(" stacks:"));
    for (int i=0; i<stackFilenames.size();i++) {
        ui->messages->append(stackFilenames.at(i));
    }
    ui->filenameCommon->setDisabled(false);
    ui->messages->setDisabled(false);
    if (stackFilenames.size() >0) {
        ui->processButton->setVisible(true);
    } else {
        ui->processButton->setVisible(false);
    }
}

void NewProjectMainWindow::processStacks()
{
    Biocytin::ZStackProjector projector;
    int nfiles = stackFilenames.size();
    ui->messages->append("Processing stacks...");
    ui->progressBar->setVisible(true);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(nfiles);
    for (int i=0; i< nfiles; i++) {
        ui->messages->append(tr("Reading ")+stackFilenames.at(i));
        ui->progressBar->setValue(i);
        QCoreApplication::processEvents();
        QString filepath = filenameDir.filePath(stackFilenames.at(i));
        //read the stack
        ZStack stack;
        stack.load(filepath.toStdString());
        //create 2D projection
        QString projFilename = filepath .remove(".tif")+".Proj.tif";
        ui->messages->append(tr("Creating a 2D projection and saving to ")+projFilename);
        QCoreApplication::processEvents();
        ZStack* proj = projector.project(
              &stack, NeuTube::IMAGE_BACKGROUND_BRIGHT, false, 0);

        proj->save(projFilename.toStdString());
        delete proj;
        /*
        ZImage image(C_Stack::width(stack), C_Stack::height(stack));
        Image_Array ima;
        ima.array = stack->array;
        switch (C_Stack::kind(stack)) {
        case GREY:
          image.setData(ima.array8);
          break;
        case COLOR:
          image.setData(ima.arrayc);
          break;
        default:
          break;
        }
        */
    }
    ui->progressBar->setVisible(false);
}

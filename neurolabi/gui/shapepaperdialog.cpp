#include "shapepaperdialog.h"
#include <QProcess>
#include <fstream>
#include "ui_shapepaperdialog.h"
#include "zdialogfactory.h"
#include "neutubeconfig.h"
#include "zparameterarray.h"
#include "zfilelist.h"
#include "zstack.hxx"
#include "zdoublevector.h"
#include "tz_math.h"
#include "zstring.h"
#include "flyem/zflyemdataframe.h"
#include "zframefactory.h"
#include "mainwindow.h"
#include "zsvggenerator.h"
#include "zdendrogram.h"

ShapePaperDialog::ShapePaperDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ShapePaperDialog), m_frame(NULL)
{
  ui->setupUi(this);
  m_objectStackDir = new ZStringParameter(
        "Object stack directory",
        (GET_DATA_DIR + "/flyem/TEM/skeletonization/session3").c_str(), this);
  m_sparseObjectDir = new ZStringParameter(
        "Sparse object directory",
        (GET_DATA_DIR + "/flyem/TEM/skeletonization/session3").c_str(), this);
  m_bundleDir = new ZStringParameter(
        "Bundle directory",
        (GET_DATA_DIR + "/flyem/TEM/data_release/bundle1").c_str(), this);
  m_resultDir = new ZStringParameter(
        "Result directory",
        (GET_DATA_DIR + "/flyem/shape_paper").c_str(), this);

  ui->sparseObjectPushButton->hide();
}

ShapePaperDialog::~ShapePaperDialog()
{
  delete ui;
}

void ShapePaperDialog::on_configPushButton_clicked()
{
  ZParameterArray parameterArray;
  parameterArray.append(m_objectStackDir);
  parameterArray.append(m_sparseObjectDir);
  parameterArray.append(m_bundleDir);
  parameterArray.append(m_resultDir);
  QDialog *dlg = ZDialogFactory::makeParameterDialog(parameterArray, this);
  dlg->exec();

  delete dlg;
}

QString ShapePaperDialog::getSparseObjectDir() const
{
  return m_sparseObjectDir->get();
}

QString ShapePaperDialog::getObjectStackDir() const
{
  return m_objectStackDir->get();
}

QString ShapePaperDialog::getDataBundlePath() const
{
  return m_bundleDir->get() + "/data_bundle.json";
}

MainWindow* ShapePaperDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(parent());
}

void ShapePaperDialog::on_sparseObjectPushButton_clicked()
{
  QString objDir = getSparseObjectDir();
  QString stackDir = getObjectStackDir();
  ZFileList fileList;
  fileList.load(stackDir.toStdString(), "tif");
  for (int i = 0; i < fileList.size(); ++i) {
    std::string filePath = fileList.getFilePath(i);
    std::string objFilePath =
        objDir.toStdString() + "/" +
        ZString::removeFileExt(ZString::getBaseName(filePath)) + ".sobj";

    if (!fexist(objFilePath.c_str())) {
      ZStack stack;
      stack.load(filePath);
      std::string offsetFilePath = filePath + ".offset.txt";
      if (fexist(offsetFilePath.c_str())) {
        ZDoubleVector vec;
        vec.importTextFile(offsetFilePath);
        if (vec.size() == 3) {
          stack.setOffset(iround(vec[0]), iround(vec[1]), iround(vec[2]));
        }

        ZObject3dScan obj;
        obj.loadStack(stack);


        obj.save(objFilePath);
      }
    }
  }
}

void ShapePaperDialog::detachFrame()
{
  m_frame = NULL;
}

void ShapePaperDialog::on_dataBundlePushButton_clicked()
{
  if (m_frame == NULL) {
    m_frame = ZFrameFactory::MakeFlyEmDataFrame(getDataBundlePath());
    if (m_frame != NULL) {
      connect(m_frame, SIGNAL(destroyed()), this, SLOT(detachFrame()));
      getMainWindow()->addFlyEmDataFrame(m_frame);
    }
  }

  if (m_frame != NULL) {
    m_frame->show();
  }
}

QString ShapePaperDialog::getSimmatPath() const
{
  return m_resultDir->get() + "/simmat.txt";
}

QString ShapePaperDialog::getDendrogramPath() const
{
  return m_resultDir->get() + "/dendrogram.svg";
}

void ShapePaperDialog::on_simmatPushButton_clicked()
{
  if (m_frame != NULL) {
    m_frame->exportSimilarityMatrix(getSimmatPath());
  }
}

void ShapePaperDialog::dump(const QString &str, bool appending)
{
  if (appending) {
    ui->messageTextEdit->append(str);
  } else {
    ui->messageTextEdit->setText(str);
  }
}

void ShapePaperDialog::on_dendrogramPushButton_clicked()
{
  if (m_frame != NULL) {
    QString simmatFile = getSimmatPath();
    if (!fexist(simmatFile.toStdString().c_str())) {
      QString targetFilePath = (GET_DATA_DIR + "/tmp/simmat.txt").c_str();
      QFile targetFile(targetFilePath);
      if (targetFile.exists()) {
        targetFile.remove();
      }

      if (QFile::copy(simmatFile, targetFilePath)) {
        QString output = getDendrogramPath();
        QProcess::execute("/Applications/MATLAB.app/bin/matlab < "
                          "/Users/zhaot/Work/SLAT/matlab/SLAT/run/flyem/tz_run_flyem_dendrogram_command.m "
                          "-nodesktop -nosplash");

        //Create name file
        std::string neuronNameFilePath = GET_DATA_DIR + "/tmp/neuron_name.txt";
        ZFlyEmDataBundle *bundle = m_frame->getDataBundle();

        std::vector<ZFlyEmNeuron> neuronArray = bundle->getNeuronArray();

        std::ofstream stream(neuronNameFilePath.c_str());
        for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
             iter != neuronArray.end(); ++iter) {
          const ZFlyEmNeuron &neuron = *iter;
          stream << neuron.getName() << std::endl;
        }
        stream.close();

        ZDendrogram dendrogram;

        ZMatrix Z;
        Z.importTextFile(GET_DATA_DIR + "/tmp/Z.txt");
        for (int i = 0; i < Z.getRowNumber(); ++i) {
          dendrogram.addLink(Z.at(i, 0), Z.at(i, 1), Z.at(i, 2) - 0.5);
        }
        dendrogram.loadLeafName(neuronNameFilePath);
        std::string svgString = dendrogram.toSvgString(15.0);

        ZSvgGenerator svgGenerator(0, 0, 1000, 6000);
        svgGenerator.write(output.toStdString().c_str(), svgString);

        dump(output + " saved.");
      }
    }
  }
}

void ShapePaperDialog::on_pushButton_5_clicked()
{
  if (m_frame != NULL) {
    m_frame->predictClass();
  }
}

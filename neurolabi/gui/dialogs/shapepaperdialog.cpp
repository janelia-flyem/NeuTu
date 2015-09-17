#include "shapepaperdialog.h"
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
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
#include "zswcglobalfeatureanalyzer.h"
#include "zmatrix.h"
#include "zobject3dscan.h"
#include "zmatlabprocess.h"

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
  updateButtonState();
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
  return qobject_cast<MainWindow*>(parent());
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
    updateButtonState();
  }
}

QString ShapePaperDialog::getConfmatPath() const
{
  return m_resultDir->get() + "/confmat.txt";
}

QString ShapePaperDialog::getSimmatPath() const
{
  return m_resultDir->get() + "/simmat.txt";
}

QString ShapePaperDialog::getFeaturePath() const
{
  return m_resultDir->get() + "/feature.csv";
}

QString ShapePaperDialog::getTrueClassLabelPath() const
{
  return m_resultDir->get() + "/true_class.txt";
}

QString ShapePaperDialog::getPredicatedClassLabelPath() const
{
  return m_resultDir->get() + "/predicated_class.txt";
}

QString ShapePaperDialog::getModelSourcePath() const
{
  return "/Users/zhaot/Work/SLAT/matlab/SLAT/run/shape_paper/tz_run_paper10_model_source.m";
}

QString ShapePaperDialog::getDendrogramPath() const
{
  return m_resultDir->get() + "/dendrogram.svg";
}

void ShapePaperDialog::exportModelSource()
{
  ZFlyEmNeuronArray *neuronArray = getNeuronArray();

  if (neuronArray != NULL) {
    QFile file(getModelSourcePath());
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);

    stream << QString("modelSourceArray = cell(%1, 1);\n").
              arg(neuronArray->size());
    //stream << QString("for k = 1:%1\n").arg(neuronArray->size());
    for (size_t i = 0; i < neuronArray->size(); ++i) {
      const ZFlyEmNeuron neuron = (*neuronArray)[i];
      stream << QString("modelSourceArray{%1} = '%2';\n").arg(i + 1).
                arg(neuron.getModelPath().c_str());
    }
    //stream << "end\n";
  }
}

void ShapePaperDialog::on_simmatPushButton_clicked()
{
  if (m_frame != NULL) {
    QFile file(getSimmatPath());
    bool calc = true;
    if (file.exists()) {
      int ret = QMessageBox::warning(
            this, "Similarity matrix exists.",
            "The similarity matrix has already been calculated. "
            "Do you want to recalculate and overwrite it?",
            QMessageBox::Yes | QMessageBox::No);
      calc = (ret == QMessageBox::Yes);
    }

    if (calc) {
      m_frame->exportSimilarityMatrix(getSimmatPath());
    }
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

void ShapePaperDialog::on_predictPushButton_clicked()
{
  ZMatrix simmat;
  simmat.importTextFile(getSimmatPath().toStdString());
  int nrows = simmat.getRowNumber();
  int ncols = simmat.getColumnNumber();

  //Normalize
  /*
  for (int i = 1; i < nrows; ++i) {
    int neuronIndex = i - 1;
    for (int j = 0; j < ncols; ++j) {
      if (neuronIndex != j) {
        simmat.set(neuronIndex, j, simmat.at(neuronIndex, j) /
                   std::max(simmat.at(neuronIndex, neuronIndex), simmat.at(j, j)));
      }
    }
  }
  */

  //Find maximum score
  int correctNumber = 0;
  for (int i = 1; i < nrows; ++i) {
    int neuronIndex = i - 1;
    double maxScore = 0.0;
    int matchedIndex = neuronIndex;
    for (int j = 0; j < ncols; ++j) {
      if (neuronIndex != j) {
        if (maxScore < simmat.at(i, j)) {
          maxScore = simmat.at(i, j);
          matchedIndex = j;
        }
      }
    }

    const ZFlyEmNeuron *neuron1 = m_frame->getNeuronFromIndex(neuronIndex);
    const ZFlyEmNeuron *neuron2 = m_frame->getNeuronFromIndex(matchedIndex);
    if (neuron1->getType() == neuron2->getType()) {
      ++correctNumber;
    } else {
      /*
      dump(QString("Wrong predction: %1 -> %2; (s = %3)").
           arg(neuron1->getName().c_str()).
           arg(neuron2->getClass().c_str()).arg(maxScore), true);
           */
    }
  }

  dump(QString("Accurate count: %1").arg(correctNumber), true);

  predictFromOrtAdjustment();
}

ZFlyEmDataBundle* ShapePaperDialog::getDataBundle()
{
  if (m_frame != NULL) {
    return m_frame->getDataBundle();
  }

  return NULL;
}

ZFlyEmNeuronArray* ShapePaperDialog::getNeuronArray()
{
  if (getDataBundle() != NULL) {
    return &(getDataBundle()->getNeuronArray());
  }

  return NULL;
}

std::map<std::string, int> ShapePaperDialog::getClassIdMap()
{
  if (getDataBundle() != NULL) {
    return getDataBundle()->getClassIdMap();
  }

  return std::map<std::string, int>();
}

void ShapePaperDialog::predictFromOrtAdjustment()
{
  ZMatrix featmat;
  featmat.importTextFile(getFeaturePath().toStdString() + ".txt");

  ZDoubleVector ratioArray(featmat.getRowNumber());
  for (size_t i = 0; i < ratioArray.size(); ++i) {
    ratioArray[i] = log(featmat.at(i, 9) / featmat.at(i, 10));
//    ratioArray[i] = log(featmat.at(i, 7));
  }

//  const double mu1 = -1.3302;
//  const double mu2 = 0.3322;
//  const double var1 = 0.2944;
//  const double var2 = 1.0380;

//  const double mu1 = -1.4074;
//  const double mu2 = 0.11518;
//  const double var1 = 0.11213;
//  const double var2 = 1.0489;

//  const double mu1 = -1.4144;
//  const double mu2 = 0.13755;
//  const double var1 = 0.11133;
//  const double var2 = 1.0146;

//  const double mu1 = -1.4083;
//  const double mu2 = 0.11153;
//  const double var1 = 0.11168;
//  const double var2 = 1.0514;

//  const double mu1 = -1.4009;
//  const double mu2 = 0.14225;
//  const double var1 = 0.11591;
//  const double var2 = 1.031;

//  const double mu1 = -1.3287;
//  const double mu2 = 0.23399;
//  const double var1 = 0.2902;
//  const double var2 = 1.1109;

  const double mu1 = -1.3304;
  const double mu2 = 0.23895;
  const double var1 = 0.28891;
  const double var2 = 1.1047;

//    const double mu1 = -1.4041;
//    const double mu2 = 0.1346;
//    const double var1 = 0.1140;
//    const double var2 = 1.0343;
//    const double mu1 = -1.111;
//    const double mu2 = 0.077144;
//    const double var1 = 0.057516;
//    const double var2 = 0.55438;

//  const double mu1 = -1.4061;
//  const double mu2 = 0.0867;
//  const double var1 = 0.1135;
//  const double var2 = 1.0798;


//  const double mu1 = -1.3813;
//  const double mu2 = 0.2738;
//  const double var1 = 0.1277;
//  const double var2 = 0.9292;

//  const double mu1 = -0.9830;
//  const double mu2 = 0.2135;
//  const double var2 = 1.0312;
//  const double var1 = 0.2932;

//  const double mu1 = -1.1103;
//  const double mu2 = 0.2637;
//  const double var1 = 0.1003;
//  const double var2 = 0.7663;


  ZMatrix simmat;
  simmat.importTextFile(getSimmatPath().toStdString());
  int nrows = simmat.getRowNumber();
  int ncols = simmat.getColumnNumber();

  //Find maximum score
  int correctNumber = 0;
  for (int i = 1; i < nrows; ++i) {
    int neuronIndex = i - 1;
    double maxScore = 0.0;
    int matchedIndex = neuronIndex;
    for (int j = 0; j < ncols; ++j) {
      if (neuronIndex != j) {
        double k = computeRatioDiff(ratioArray[neuronIndex],
                                            ratioArray[j], mu1, var1,
                                            mu2, var2);
        double score = simmat.at(i, j) * sqrt(k); /// (1 + exp((0.5 - k) * 5));
        if (maxScore < score) {
          maxScore = score;
          matchedIndex = j;
        }
      }
    }

    const ZFlyEmNeuron *neuron1 = m_frame->getNeuronFromIndex(neuronIndex);
    const ZFlyEmNeuron *neuron2 = m_frame->getNeuronFromIndex(matchedIndex);
    if (neuron1->getType() == neuron2->getType()) {
      ++correctNumber;
    } else {
      /*
      dump(QString("Wrong predction: %1 -> %2; (s = %3)").
           arg(neuron1->getName().c_str()).
           arg(neuron2->getClass().c_str()).arg(maxScore), true);
           */
    }
  }

  dump(QString("Accurate count (ratio adjusted): %1").arg(correctNumber), true);
}

void ShapePaperDialog::computeFeatureMatrix()
{
  //Compute features
  if (m_frame != NULL) {
    ZSwcGlobalFeatureAnalyzer::EFeatureSet setName =
        ZSwcGlobalFeatureAnalyzer::NGF3;
    ZFlyEmDataBundle *bundle = m_frame->getDataBundle();
    ZFlyEmNeuronArray &neuronArray = bundle->getNeuronArray();
    ZMatrix featmat(neuronArray.size(),
                    ZSwcGlobalFeatureAnalyzer::getFeatureNumber(setName));
    int row = 0;
    std::vector<std::string> neuronName;
    for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
         iter != neuronArray.end(); ++iter, ++row) {
      ZFlyEmNeuron &neuron = *iter;
      std::vector<double> featureSet =
          ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
            *(neuron.getModel()), setName);
      neuronName.push_back(neuron.getName());
      featmat.setRowValue(row, featureSet);
    }

    featmat.exportCsv(getFeaturePath().toStdString() + ".txt");
    featmat.exportCsv(getFeaturePath().toStdString(), neuronName,
                      ZSwcGlobalFeatureAnalyzer::getFeatureNameArray(setName));
    dump(getFeaturePath() + " saved.", true);
  }
}

void ShapePaperDialog::on_featurePushButton_clicked()
{
  computeFeatureMatrix();
}

double ShapePaperDialog::computeRatioDiff(
    double x, double y, double mu1, double var1, double mu2, double var2)
{
  double zx1 = (x - mu1) * (x - mu1) / var1 / 2.0;
  double zx2 = (x - mu2) * (x - mu2) / var2 / 2.0;
  double zy1 = (y - mu1) * (y - mu1) / var1 / 2.0;
  double zy2 = (y - mu2) * (y - mu2) / var2 / 2.0;
  double rx = exp(zx2 - zx1) * sqrt(var2 / var1);
  double ry = exp(zy2 - zy1) * sqrt(var2 / var1);

  double k = 1.0;
  if (rx > 0.0 || ry > 0.0) {
    double t = (rx * ry + 1) / (rx + ry);
    k = t / (1 + t);
  }

  return k;

      /*
  return std::fabs((x - y) * ((x + y - mu1 - mu1) /var1 -
                   (x + y - mu2 - mu2)/var2));
                       */
}

void ShapePaperDialog::updateButtonState()
{
  ui->simmatPushButton->setEnabled(m_frame != NULL);
}

void ShapePaperDialog::computeConfusionMatrix()
{

}

QString ShapePaperDialog::getPath(EResult result) const
{
  switch (result) {
  case RESULT_SIMMAT:
    return getSimmatPath();
  case RESULT_FEATMAT:
    return getFeaturePath();
  case RESULT_DENDROGRAM:
    return getDendrogramPath();
  case RESULT_TRUE_CLASS_LABEL:
    return getTrueClassLabelPath();
  case RESULT_PRED_CLASS_LABEL:
    return getPredicatedClassLabelPath();
  case RESULT_CONFMAT:
    return getConfmatPath();
  case RESULT_LAYER_FEATMAT:
    return m_resultDir->get() + "/layer_feat.txt";
  case RESULT_MODEL_SOURCE:
    return getModelSourcePath();
  case RESULT_CLUSTERING:
    return m_resultDir->get() + "/clustering.txt";
  case RESULT_NEURON_INFO:
    return m_resultDir->get() + "/neuron_info.json";
  }

  return QString();
}

bool ShapePaperDialog::exists(EResult result) const
{
  QString path = getPath(result);

  return QFile(path).exists();
}

void ShapePaperDialog::exportClassLabel()
{

  ZFlyEmNeuronArray *neuronArray = getNeuronArray();
  if (neuronArray != NULL) {
    std::vector<int> idArray;
    std::map<std::string, int> classIdMap = getClassIdMap();
    //foreach neuron
    for (ZFlyEmNeuronArray::const_iterator iter = neuronArray->begin();
         iter != neuronArray->end(); ++iter) {
      //get class label
      const ZFlyEmNeuron neuron = *iter;
      int id = classIdMap[neuron.getType()];
      idArray.push_back(id);
    }

    //Save labels to a text file
    QString filePath = getTrueClassLabelPath();
    std::ofstream stream;
    stream.open(filePath.toStdString().c_str());
    for (size_t i = 0; i < idArray.size(); ++i) {
      stream << idArray[i] << std::endl;
    }
    stream.close();
    dump(filePath + " saved.", true);
  }

}

void ShapePaperDialog::exportNeuronInfo()
{
  ZFlyEmNeuronArray *neuronArray = getNeuronArray();
  if (neuronArray != NULL) {
    std::map<std::string, int> classIdMap = getClassIdMap();
    //foreach neuron
    int index = 1;
    ZJsonArray neuronArrayObj;

    for (ZFlyEmNeuronArray::const_iterator iter = neuronArray->begin();
         iter != neuronArray->end(); ++iter, ++index) {
      const ZFlyEmNeuron neuron = *iter;

      ZJsonObject neuronObj;
      neuronObj.setEntry("index", index);
      neuronObj.setEntry("class", classIdMap[neuron.getType()]);
      neuronObj.setEntry("id", neuron.getId());
      neuronObj.setEntry("name", neuron.getName());
      neuronObj.setEntry("path", neuron.getModelPath());

      neuronArrayObj.append(neuronObj);
    }

    ZJsonArray classArrayObj;
    for (std::map<std::string, int>::const_iterator iter = classIdMap.begin();
         iter != classIdMap.end(); ++iter) {
      ZJsonObject classMapObj;
      classMapObj.setEntry("label", iter->second);
      classMapObj.setEntry("name", iter->first);
      classArrayObj.append(classMapObj);
    }

    ZJsonObject headObj;
    headObj.setEntry("neuron", neuronArrayObj);
    headObj.setEntry("class", classArrayObj);

    headObj.dump(getPath(RESULT_NEURON_INFO).toStdString());
  }
}

void ShapePaperDialog::tryOutput(EResult result)
{
  if (!exists(result) && (m_frame != NULL)) {
    switch(result) {
    case RESULT_SIMMAT:
      m_frame->exportSimilarityMatrix(getSimmatPath());
      break;
    case RESULT_FEATMAT:
      computeFeatureMatrix();
      break;
    case RESULT_TRUE_CLASS_LABEL:
      exportClassLabel();
      break;
    case RESULT_CONFMAT:
      computeConfusionMatrix();
      break;
    case RESULT_LAYER_FEATMAT:
      computeLayerFeature();
      break;
    case RESULT_MODEL_SOURCE:
      exportModelSource();
      break;
    case RESULT_NEURON_INFO:
      exportNeuronInfo();
      break;
    default:
      break;
    }
  }
}

void ShapePaperDialog::computeLayerFeature()
{
  if (m_frame != NULL) {
    m_frame->exportLayerFeature(getPath(RESULT_LAYER_FEATMAT));
  }
}

void ShapePaperDialog::on_allResultPushButton_clicked()
{
  if (m_frame == NULL) {
    dump("No data available.");
    return;
  }

  //Generate similarity matrix
  tryOutput(RESULT_SIMMAT);

  //Generate neuron class labels
  tryOutput(RESULT_TRUE_CLASS_LABEL);

  tryOutput(RESULT_CONFMAT);

  //Generate neuron IDs

  //Generate neuron Names

  //Generate feature matrix
  tryOutput(RESULT_PRED_CLASS_LABEL);

  //Generate confusion matrix

  //Generate layer features
  tryOutput(RESULT_LAYER_FEATMAT);

  tryOutput(RESULT_MODEL_SOURCE);

  tryOutput(RESULT_NEURON_INFO);

  dump("Done");
}

void ShapePaperDialog::on_clusteringPushButton_clicked()
{
  if (m_frame != NULL) {
    QString simmatFile = getSimmatPath();
    if (!simmatFile.isEmpty()) {
      QString targetFilePath((GET_DATA_DIR + "/tmp/simmat.txt").c_str());
      QFile targetFile(targetFilePath);
      if (targetFile.exists()) {
        targetFile.remove();
      }

      if (QFile::copy(simmatFile, targetFilePath)) {
        ZMatlabProcess process;
        if (process.findMatlab()) {
          process.setScript("/Users/zhaot/Work/SLAT/matlab/SLAT/run/"
                            "shape_paper/tz_run_paper10_clustering_command.m");
          if (process.run()) {
            const ZJsonObject &output = process.getOutput();
            if (output.hasKey("cluster_file")) {
              const char *outputFile =
                  ZJsonParser::stringValue(output["cluster_file"]);
              if (outputFile != NULL) {
                QFile file(getPath(RESULT_CLUSTERING));
                if (file.exists()) {
                  file.remove();
                }

                QFile::copy(outputFile, getPath(RESULT_CLUSTERING));
                dump(getPath(RESULT_CLUSTERING) + " saved.");

                ZMatrix mat;
                mat.importTextFile(getPath(RESULT_CLUSTERING).toStdString());

                QString examplarDirPath = m_resultDir->get() + "/examplars";
                QStringList nameFilter("*.swc");
                QDir examplarDir(examplarDirPath);
                QStringList oldSwcFileList = examplarDir.entryList(nameFilter);
                foreach (QString swcFile, oldSwcFileList) {
                  QFile(examplarDir.absoluteFilePath(swcFile)).remove();
                }

                ZFlyEmNeuronArray *neuronArray = getNeuronArray();
                for (int i = 0; i < mat.getRowNumber(); ++i) {
                  int isExamplar = iround(mat.at(i, 1));
                  if (isExamplar == 1) {
                    ZFlyEmNeuron &neuron = (*neuronArray)[i];
                    neuron.getModel()->save(examplarDirPath.toStdString() +
                          "/" + ZString::getBaseName(neuron.getModelPath()));
                  }
                }
                dump(examplarDirPath + " updated.", true);
              } else {
                dump("Cannot finish the task for unknown reasons.");
              }
            } else {
              dump("No output key found.");
            }
          } else {
            dump("Cannot finish the task for unknown reasons.");
          }
        } else {
          dump("No Matlab found. This function requires Matlab.");
        }
        /*
        QProcess::execute(
              "/Applications/MATLAB.app/bin/matlab < "
              "/Users/zhaot/Work/SLAT/matlab/SLAT/run/flyem/tz_run_flyem_clustering_command.m "
              "-nodesktop -nosplash");

        frame->assignClass(GET_DATA_DIR + "/tmp/labels.txt");
        */
      } else {
        dump("Unable to generate similarity matrix");
      }
    }
  }
}

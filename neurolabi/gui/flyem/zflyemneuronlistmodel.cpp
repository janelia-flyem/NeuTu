#include "zflyemneuronlistmodel.h"
#include <QModelIndex>
#include "zflyemneuron.h"
#include "zstackdoc.h"
#include "zflyemneuronpresenter.h"
#include "zswcobjsmodel.h"
//#include "zpunctaobjsmodel.h"
#include "zswccolorscheme.h"
#include "QsLog.h"
#include "zobject3dscanarray.h"
#include "zstackfactory.h"
#include "zlabelcolortable.h"
#include "zscalablestack.h"

ZFlyEmNeuronListModel::ZFlyEmNeuronListModel(QObject *parent) :
  QAbstractTableModel(parent)
{
  m_defaultPresenter = new ZFlyEmNeuronInfoPresenter(this);
  m_presenter = m_defaultPresenter;

  /*
  ZFlyEmNeuron *neuron  = new ZFlyEmNeuron;
  m_neuronList.append(neuron);
  m_neuronList.append(neuron);
  */
}

int ZFlyEmNeuronListModel::columnCount(const QModelIndex &/*parent*/) const
{
  return m_presenter->columnCount();
}

int ZFlyEmNeuronListModel::rowCount(const QModelIndex &/*parent*/) const
{
  return m_neuronList.size();
}

QVariant ZFlyEmNeuronListModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  if (index.row() >= m_neuronList.size() || index.row() < 0) {
    return QVariant();
  }

  const ZFlyEmNeuron *neuron = getNeuron(index.row());
  if (neuron != NULL) {
    return m_presenter->data(*neuron, index.column(), role);
  }

/*
  if (role == Qt::DisplayRole) {
    const ZFlyEmNeuron *neuron = getNeuron(index);
    switch (index.column()) {
    case 0:
    {
      QString data = QString("%1").arg(neuron->getId());

      return data;
    }
    case 1:
      return neuron->getName().c_str();
    case 2:
      return neuron->getClass().c_str();
    case 3:
      if (neuron->getMatched() == NULL) {
        return "";
      } else {
        return QString(":") + neuron->getMatched()->getClass().c_str() +
            QString(" (%1)").arg(neuron->getMatched()->getId());
      }
    case 4:
      return neuron->getModelPath().c_str();
    default:
      return QVariant();
    }
  } else if (role == Qt::ToolTipRole) {
    const ZFlyEmNeuron *neuron = getNeuron(index);
    if (neuron != NULL) {
      QString data = QString("%1").arg(neuron->getId()) +
          " (" + neuron->getModelPath().c_str() + ")";
      return data;
    }
  }
  */

  return QVariant();
}

QVariant ZFlyEmNeuronListModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{

  return m_presenter->headerData(section, orientation, role);

  /*
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case 0:
        return "ID";
      case 1:
        return "Name";
      case 2:
        return "Class";
      case 3:
        return "Predicted";
      case 4:
        return "Model Path";
      default:
        return QVariant();
      }
    }
  }

  return QVariant();
  */
}


void ZFlyEmNeuronListModel::clear()
{
  removeRows(0, rowCount());
  m_neuronList.clear();
}

void ZFlyEmNeuronListModel::append(const ZFlyEmNeuron *neuron)
{
  m_neuronList.append(neuron);
  insertRow(m_neuronList.size() - 1);

  /*
  emit dataChanged(index(0, 0),
                   index(m_neuronList.size(), columnCount()));
                   */
}

const ZFlyEmNeuron* ZFlyEmNeuronListModel::getNeuron(int index) const
{
  if (index < 0 || index >= rowCount()) {
    return NULL;
  }

  return m_neuronList[index];
}

const ZFlyEmNeuron* ZFlyEmNeuronListModel::getNeuron(int row, int col) const
{
  if (row < 0 || row >= rowCount()) {
    return NULL;
  }

  const ZFlyEmNeuron *neuron = m_neuronList[row];

  if (neuron != NULL) {
    return m_presenter->getNeuron(*neuron, col);
  }

  return NULL;
}

const ZFlyEmNeuron* ZFlyEmNeuronListModel::getNeuron(
    const QModelIndex &index) const
{
  return getNeuron(index.row(), index.column());
}

ZFlyEmNeuron* ZFlyEmNeuronListModel::getNeuron(const QModelIndex &index)
{
  return const_cast<ZFlyEmNeuron*>(
        static_cast<const ZFlyEmNeuronListModel&>(*this).getNeuron(index));
}

QVector<ZFlyEmNeuron*> ZFlyEmNeuronListModel::getNeuronArray(
    const QModelIndexList &indexList)
{
  QVector<ZFlyEmNeuron*> neuronArray;
  foreach (const QModelIndex &index, indexList) {
    if (index.row() >= 0 || index.row() < rowCount()) {
      neuronArray.append(const_cast<ZFlyEmNeuron*>(m_neuronList[index.row()]));
    }
  }

  return neuronArray;
}

QVector<const ZFlyEmNeuron*> ZFlyEmNeuronListModel::getNeuronArray(
    const QModelIndex &index) const
{
  if (index.row() < 0 || index.row() >= rowCount()) {
    return QVector<const ZFlyEmNeuron*>();
  }

  const ZFlyEmNeuron *neuron = m_neuronList[index.row()];

  if (neuron != NULL) {
    return m_presenter->getNeuronArray(*neuron, index.column());
  }

  return QVector<const ZFlyEmNeuron*>();
}

void ZFlyEmNeuronListModel::retrieveModel(
    const QModelIndexList &indexList, ZStackDoc *doc, bool forceUpdate) const
{
//  doc->blockSignals(true);

  doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  QVector<const ZFlyEmNeuron*> neuronArray;
  QMap<std::string, QColor> colorMap;
  ZSwcColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);

  foreach (QModelIndex index, indexList) {
    QVector<const ZFlyEmNeuron*> subArray = getNeuronArray(index);
    foreach (const ZFlyEmNeuron *neuron, subArray) {
      neuronArray.append(neuron);

      if (colorMap.count(neuron->getType()) == 0) {
        colorMap[neuron->getType()] = colorScheme.getColor(colorMap.size());
      }
    }
  }

  qDebug() << colorMap;

  foreach (const ZFlyEmNeuron *neuron, neuronArray) {
    if (neuron != NULL) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      if (forceUpdate) {
        neuron->updateDvidModel(forceUpdate);
      }

      ZSwcTree *model = neuron->getModel();

      if (model != NULL) {
        ZSwcTree *tree = model->clone();
        //qDebug() << neuron->getClass();
        //qDebug() << colorMap[neuron->getClass()];
        //qDebug() << colorMap["L1"];
        tree->setColor(colorMap[neuron->getType()]);
        doc->addObject(tree, true);
      }

      std::vector<ZPunctum*> puncta = neuron->getSynapse();
      for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
           iter != puncta.end(); ++iter) {
        doc->addObject(*iter, false);
      }
    }
  }

  doc->endObjectModifiedMode();
  doc->notifyObjectModified();
//  doc->blockSignals(false);
//  doc->swcObjsModel()->updateModelData();
//  doc->punctaObjsModel()->updateModelData();
}

ZScalableStack* ZFlyEmNeuronListModel::retrieveBody(
    const QModelIndexList &indexList) const
{
  QVector<const ZFlyEmNeuron*> neuronArray;

  foreach (QModelIndex index, indexList) {
    QVector<const ZFlyEmNeuron*> subArray = getNeuronArray(index);
    foreach (const ZFlyEmNeuron *neuron, subArray) {
      neuronArray.append(neuron);
    }
  }

  ZObject3dScanArray bodyArray;

  foreach (const ZFlyEmNeuron *neuron, neuronArray) {
    if (neuron != NULL) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZObject3dScan *body = neuron->getBody();

      if (body != NULL) {
        bodyArray.push_back(*body);
      }
    }
  }

  ZScalableStack *stack = NULL;

  if (!bodyArray.empty()) {
    const size_t maxVolume = 1024*1024*100 / bodyArray.size();

    ZIntPoint dsIntvPt;

    if (bodyArray.getBoundBox().getVolume() > maxVolume) {
      int dsIntv =
          iround(Cube_Root(
                   static_cast<double>(bodyArray.getBoundBox().getVolume()) /
                   maxVolume)) - 1;
      bodyArray.downsample(dsIntv, dsIntv, dsIntv);
      dsIntvPt.set(dsIntv, dsIntv, dsIntv);
    }

    ZLabelColorTable colorTable;
    for (size_t i = 0; i < bodyArray.size(); ++i) {
      ZObject3dScan &obj = bodyArray[i];
      obj.setColor(colorTable.getColor(i));
    }
    if (bodyArray.size() == 1) {
      stack = new ZScalableStack(
            ZStackFactory::MakeBinaryStack(bodyArray, 255));
    } else {
      stack = new ZScalableStack(ZStackFactory::MakeColorStack(bodyArray));
    }

    stack->setScale(
          dsIntvPt.getX() + 1, dsIntvPt.getY() + 1, dsIntvPt.getZ() + 1);
  }

  return stack;
}

ZIntPoint ZFlyEmNeuronListModel::retrieveBody(
    const QModelIndexList &indexList, ZStackDoc *doc) const
{
  QVector<const ZFlyEmNeuron*> neuronArray;

  foreach (QModelIndex index, indexList) {
    QVector<const ZFlyEmNeuron*> subArray = getNeuronArray(index);
    foreach (const ZFlyEmNeuron *neuron, subArray) {
      neuronArray.append(neuron);
    }
  }

  ZObject3dScanArray bodyArray;

  foreach (const ZFlyEmNeuron *neuron, neuronArray) {
    if (neuron != NULL) {
      //doc->addSwcTree(neuron->getModel()->clone(), true);
      ZObject3dScan *body = neuron->getBody();

      if (body != NULL) {
        bodyArray.push_back(*body);
      }
    }
  }

  ZStack *stack = NULL;

  ZIntPoint dsIntvPt;
  if (!bodyArray.empty()) {
    const size_t maxVolume = 1024*1024*100 / bodyArray.size();


    if (bodyArray.getBoundBox().getVolume() > maxVolume) {
      int dsIntv =
          iround(Cube_Root(
                   static_cast<double>(bodyArray.getBoundBox().getVolume()) /
                   maxVolume)) - 1;
      bodyArray.downsample(dsIntv, dsIntv, dsIntv);
      dsIntvPt.set(dsIntv, dsIntv, dsIntv);
    }

    ZLabelColorTable colorTable;
    for (size_t i = 0; i < bodyArray.size(); ++i) {
      ZObject3dScan &obj = bodyArray[i];
      obj.setColor(colorTable.getColor(i));
    }

    //ZStack *stack = bodyArray.toStackObject();


    if (bodyArray.size() == 1) {
      stack = ZStackFactory::MakeBinaryStack(bodyArray, 255);
    } else {
      stack = ZStackFactory::MakeColorStack(bodyArray);
    }
  }

  if (stack != NULL) {
    doc->loadStack(stack);
    doc->setTag(NeuTube::Document::FLYEM_BODY);
  }

  return dsIntvPt;
}


bool ZFlyEmNeuronListModel::insertRows(
    int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  endInsertRows();

  return true;
}

bool ZFlyEmNeuronListModel::insertColumns(
    int col, int count, const QModelIndex &parent)
{
  beginInsertColumns(parent, col, col + count - 1);
  endInsertColumns();

  return true;
}

bool ZFlyEmNeuronListModel::removeRows(
    int row, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    endRemoveRows();

    return true;
  }

  return false;
}

bool ZFlyEmNeuronListModel::removeColumns(
    int col, int count, const QModelIndex &parent)
{
  if (count > 0) {
    beginRemoveColumns(parent, col, col + count - 1);
    endRemoveColumns();

    return true;
  }

  return false;
}

void ZFlyEmNeuronListModel::exportCsv(const QString &path)
{
  QFile file(path);

  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    for (int row =0; row < rowCount(); ++row) {
      for (int col = 0; col < columnCount(); ++col) {
        QVariant value = data(createIndex(row, col));
        if (value.type() == QVariant::String) {
          stream << "\"";
        }
        stream << value.toString();

        if (value.type() == QVariant::String) {
          stream << "\"";
        }

        if (col < columnCount() - 1) {
          stream << ",";
        } else {
          stream << endl;
        }
      }
    }

    file.close();
  }
}

void ZFlyEmNeuronListModel::exportSwc(
    const QString &path, ZFlyEmCoordinateConverter::ESpace coordSpace)
{
  foreach (const ZFlyEmNeuron *neuron, m_neuronList) {
    ZSwcTree *tree = NULL;

    switch (coordSpace) {
    case ZFlyEmCoordinateConverter::PHYSICAL_SPACE:
      tree = neuron->getModel();
      break;
    case ZFlyEmCoordinateConverter::IMAGE_SPACE:
      tree = neuron->getUnscaledModel();
      break;
    default:
      break;
    }

    if (tree != NULL) {
      QDir dir(path);

      QString fileName = QString("%1.swc").arg(neuron->getId());

      if (!neuron->getType().empty()) {
        fileName = QString("%1_").arg(neuron->getType().c_str()) + fileName;
        fileName.replace('/', "_or_");
      }

      tree->save(dir.absoluteFilePath(fileName).toStdString());
    }
  }
}

void ZFlyEmNeuronListModel::setPresenter(ZFlyEmNeuronPresenter *presenter)
{
  if (m_presenter != presenter) {
    int oldColumnCount = m_presenter->columnCount();
    if (presenter == NULL) {
      m_presenter = m_defaultPresenter;
    } else {
      m_presenter = presenter;
    }

    int newColumnCount = m_presenter->columnCount();

    int colDiff = newColumnCount - oldColumnCount;
    if (colDiff < 0) {
      removeColumns(newColumnCount, -colDiff);
    } else if (colDiff > 0) {
      insertColumns(oldColumnCount, colDiff);
    }
  }
}

void ZFlyEmNeuronListModel::notifyDataChanged (
    const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
  emit dataChanged(topLeft, bottomRight);
}

void ZFlyEmNeuronListModel::notifyAllDataChanged()
{
  QModelIndex topLeft = createIndex(0, 0);
  QModelIndex bottomRight = createIndex(rowCount() - 1, columnCount() - 1);

  emit dataChanged(topLeft, bottomRight);
}

void ZFlyEmNeuronListModel::notifyRowDataChanged(int row)
{
  QModelIndex topLeft = createIndex(row, 0);
  QModelIndex bottomRight = createIndex(row, columnCount() - 1);

  emit dataChanged(topLeft, bottomRight);
}

bool ZFlyEmNeuronListModel::isNeuronKey(const QModelIndex &index)
{
  return index.column() == 0;
}

QString ZFlyEmNeuronListModel::getColumnName(int col) const
{
  return m_presenter->getColumnName(col);
}

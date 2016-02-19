#include "zsegmentationproject.h"
#include <QString>
#include <QFileInfo>
#include <QDir>

#include "zstack.hxx"
#include "zstackdocreader.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "zfiletype.h"
#include "zlabelcolortable.h"
#include "zstackview.h"
#include "zobject3dscanarray.h"

ZSegmentationProject::ZSegmentationProject(QObject *parent) :
  QObject(parent), m_stack(NULL), m_dataFrame(NULL)
{
}

ZSegmentationProject::~ZSegmentationProject()
{
  clear();
}

void ZSegmentationProject::clear(bool deletingFrame)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    if (deletingFrame) {
      delete m_dataFrame;
    }

    m_dataFrame = NULL;
  }

  delete m_stack;
  m_stack = NULL;
  m_labelTree.clear();
  m_source.clear();
}

ZTreeNode<ZObject3dScan>* ZSegmentationProject::getRootLabel() const
{
  return const_cast<ZTreeNode<ZObject3dScan>*>(m_labelTree.getRoot());
}

void ZSegmentationProject::loadStack(const QString &fileName)
{
  delete m_stack;
  m_stack = new ZStack;
  m_stack->load(fileName.toStdString());

  m_dataFrame->document()->loadStack(m_stack->clone());

  ZTreeNode<ZObject3dScan> *root = new ZTreeNode<ZObject3dScan>;
  m_labelTree.setRoot(root);

  if (ZFileType::fileType(fileName.toStdString()) == ZFileType::JSON_FILE) {
    ZJsonObject dataJson;
    dataJson.load(fileName.toStdString());
    if (dataJson.hasKey("segmentation")) {
      ZJsonObject nodeJson(
            dataJson["segmentation"], ZJsonValue::SET_INCREASE_REF_COUNT);
      loadJsonNode(root, nodeJson,
                   QFileInfo(fileName).absoluteDir().absolutePath());
    }
    m_source = fileName;
  }
}

void ZSegmentationProject::generateTestData()
{
  m_stack = new ZStack;
  m_stack->load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/em_stack_slice.tif");

  m_dataFrame->document()->loadStack(m_stack->clone());

  ZTreeNode<ZObject3dScan> *root = new ZTreeNode<ZObject3dScan>;
  m_labelTree.setRoot(root);

//  ZTreeNode<ZObject3dScan> *node = new ZTreeNode<ZObject3dScan>;
//  node->data().setLabel(1);
//  node->setParent(root);

//  node = new ZTreeNode<ZObject3dScan>;
//  node->data().setLabel(2);
//  node->setParent(root);

//  ZTreeNode<ZObject3dScan> *node2 = new ZTreeNode<ZObject3dScan>;
//  node2->data().setLabel(1);
//  node2->setParent(node);

//  node2 = new ZTreeNode<ZObject3dScan>;
//  node2->data().setLabel(2);
//  node2->setParent(node);

//  node2 = new ZTreeNode<ZObject3dScan>;
//  node2->data().setLabel(3);
//  node2->setParent(node);
}

void ZSegmentationProject::loadSegmentationTarget(
    ZTreeNode<ZObject3dScan> *node)
{
  ZStackDocReader docReader;
  if (node != NULL) {
    const ZObject3dScan &obj = node->data();
    if (!obj.isEmpty()) {
      ZStack *substack = obj.toStackObject();
      size_t offset = 0;
      for (int z = 0; z < substack->depth(); ++z) {
        for (int y = 0; y < substack->height(); ++y) {
          for (int x = 0; x < substack->width(); ++x) {
            if (substack->array8()[offset] == 1) {
              int v = m_stack->getIntValue(x + substack->getOffset().getX(),
                                           y + substack->getOffset().getY(),
                                           z + substack->getOffset().getZ());
              substack->array8()[offset] = v;
            }
            ++offset;
          }
        }
      }

      docReader.setStack(substack);
    } else {
      if (node->isRoot()) {
        docReader.setStack(m_stack->clone());
      }
    }
  }

  //Add child objects
  ZTreeNode<ZObject3dScan> *child = node->firstChild();
  ZLabelColorTable colorTable;
  while (child != NULL) {
    ZObject3dScan *obj = new ZObject3dScan;
    *obj = child->data();
    QColor color = colorTable.getColor(obj->getLabel());
    color.setAlpha(32);
    obj->setColor(color);
    obj->setZOrder(100);
    docReader.addObject(obj);
    child = child->nextSibling();
  }

  if (docReader.hasData()) {
    setDocData(docReader);
  }
}

void ZSegmentationProject::setDocData(ZStackDocReader &reader)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->document()->reloadData(reader);
    m_dataFrame->document()->notify3DGraphModified();
    m_dataFrame->view()->setSliceIndex(
          m_dataFrame->document()->getStackSize().getZ() / 2);
  }
}

void ZSegmentationProject::setDataFrame(ZStackFrame *frame)
{
  m_dataFrame = frame;
  connect(m_dataFrame, SIGNAL(closed(ZStackFrame*)), this, SLOT(detachFrame()));
}

void ZSegmentationProject::loadJsonNode(
    ZTreeNode<ZObject3dScan> *parent, const ZJsonObject &nodeJson,
    const QString &rootDir)
{
  if (nodeJson.hasKey("source")) {
    ZTreeNode<ZObject3dScan> *node = new ZTreeNode<ZObject3dScan>();
    QString sourceFile(ZJsonParser::stringValue(nodeJson["source"]));

    QFileInfo fileInfo(sourceFile);
    if (!fileInfo.isAbsolute()) {
      QDir dir(rootDir);
      sourceFile = dir.absoluteFilePath(sourceFile);
    }

//    ZObject3dScan obj;
//    node->setData(obj);

    node->data().load(sourceFile.toStdString());
    if (nodeJson.hasKey("label")) {
      node->data().setLabel(ZJsonParser::integerValue(nodeJson["label"]));
    }

    node->setParent(parent);
    parent = node;
  }

  if (nodeJson.hasKey("child")) {
    ZJsonArray childJson(
          nodeJson["child"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < childJson.size(); ++i) {
      ZJsonObject childNodeJson(
            childJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      loadJsonNode(parent, childNodeJson, rootDir);
    }
  }
}

ZJsonObject ZSegmentationProject::getNodeJson(
    const ZTreeNode<ZObject3dScan> *node)
{
  ZJsonObject jsonObject;

  if (!node->data().getSource().empty()) {
    jsonObject.setEntry("source", node->data().getSource());
    jsonObject.setEntry("label", (int) node->data().getLabel());
  }

  //For each child
  ZTreeNode<ZObject3dScan> *child = node->firstChild();
  ZJsonArray jsonChildren;
  while (child != NULL) {
    //Add to the node array
    jsonChildren.append(getNodeJson(child));
    child = child->nextSibling();
  }

  jsonObject.setEntry("child", jsonChildren);

  return jsonObject;
}

void ZSegmentationProject::save(const QString &fileName)
{
  if (!fileName.isEmpty()) {
    ZJsonObject projectJson;

    QFileInfo fileInfo(fileName);

    QDir saveDir(fileInfo.absoluteDir());

    qDebug() << "Saving directory: " << saveDir.absolutePath();

    //saveDir.mkdir(".");

    QString signalPath = saveDir.absoluteFilePath("signal.tif");

    ZJsonObject stackJson;
    stackJson.setEntry("url", signalPath.toStdString());
    stackJson.setEntry("type", std::string("single"));

    projectJson.setEntry("zstack", stackJson);

    qDebug() <<"Saving signal: " << signalPath;
    m_stack->save(signalPath.toStdString());

    //Saving regions
    ZTreeIterator<ZObject3dScan> iterator(m_labelTree);
    while (iterator.hasNext()) {
      ZTreeNode<ZObject3dScan> *node = iterator.nextNode();
      if (!node->isRoot()) {
        QString parentSource = node->parent()->data().getSource().c_str();
        parentSource = parentSource.remove(".sobj");
        QString output = QString("%1_%2.sobj").arg(parentSource).arg(
              node->data().getLabel());
        node->data().save(saveDir.absoluteFilePath(output).toStdString());
        node->data().setSource(output.toStdString());
      }
    }

    ZJsonObject labelJson = getNodeJson(m_labelTree.getRoot());
    projectJson.setEntry("segmentation", labelJson);

    qDebug() << "Saving project: " << fileName;
    projectJson.dump(fileName.toStdString());

    m_source = fileName;
  }
}

void ZSegmentationProject::detachFrame()
{
  m_dataFrame = NULL;
}

void ZSegmentationProject::exportLeafObjects(const QString &dirName)
{
  if (!dirName.isEmpty()) {
    //Saving regions
    ZTreeIterator<ZObject3dScan> iterator(m_labelTree);
    while (iterator.hasNext()) {
      ZTreeNode<ZObject3dScan> *node = iterator.nextNode();
      if (node->isLeaf()) {
        QString source = node->data().getSource().c_str();
        if (!source.isEmpty()) {
          QString output;
          QFileInfo fileInfo(source);
          QDir dir(dirName);
          output = dir.absoluteFilePath(fileInfo.fileName());
          node->data().save(output.toStdString());
        }
      }
    }
  }
}

void ZSegmentationProject::exportLabelField(const QString &fileName)
{
  ZObject3dScanArray objArray;

  ZTreeIterator<ZObject3dScan> iterator(m_labelTree);
  while (iterator.hasNext()) {
    ZTreeNode<ZObject3dScan> *node = iterator.nextNode();
    if (node->isLeaf()) {
      objArray.push_back(node->data());
    }
  }

  ZStack *stack = objArray.toStackObject();
  stack->save(fileName.toStdString());

  delete stack;
}

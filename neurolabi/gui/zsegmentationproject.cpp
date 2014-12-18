#include "zsegmentationproject.h"
#include <QString>
#include <QFileInfo>

#include "zstack.hxx"
#include "zstackdocreader.h"
#include "zstackframe.h"
#include "zstackdoc.h"

ZSegmentationProject::ZSegmentationProject(QObject *parent) :
  QObject(parent), m_stack(NULL)
{
}

ZSegmentationProject::~ZSegmentationProject()
{
  delete m_stack;
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

      ZStackDocReader docReader;
      docReader.setStack(substack);
      setDocData(docReader);
    } else {
      if (node->isRoot()) {
        ZStackDocReader docReader;
        docReader.setStack(m_stack->clone());
        setDocData(docReader);
      }
    }
  }
}

void ZSegmentationProject::setDocData(ZStackDocReader &reader)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->document()->reloadData(reader);
  }
}

void ZSegmentationProject::setDataFrame(ZStackFrame *frame)
{
  m_dataFrame = frame;
}

ZJsonObject ZSegmentationProject::getNodeJson(
    const ZTreeNode<ZObject3dScan> *node)
{
  ZJsonObject jsonObject;

  if (!node->data().getSource().empty()) {
    jsonObject.setEntry("source", node->data().getSource());
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
    projectJson.setEntry("stack", signalPath.toStdString());

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
    projectJson.setEntry("label", labelJson);

    qDebug() << "Saving project: " << fileName;
    projectJson.dump(fileName.toStdString());
  }
}

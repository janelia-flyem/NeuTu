#ifndef ZSEGMENTATIONPROJECT_H
#define ZSEGMENTATIONPROJECT_H

#include <QObject>
#include "ztree.h"
#include "zobject3dscan.h"

class ZStack;
class ZStackFrame;
class ZStackDocReader;
class ZJsonObject;

class ZSegmentationProject : public QObject
{
  Q_OBJECT
public:
  explicit ZSegmentationProject(QObject *parent = 0);
  ~ZSegmentationProject();

  ZTreeNode<ZObject3dScan>* getRootLabel() const;

  void setStack(ZStack *stack) {
    m_stack = stack;
  }

  void generateTestData();

  void loadSegmentationTarget(ZTreeNode<ZObject3dScan> *node);

  void setDocData(ZStackDocReader &reader);

  void setDataFrame(ZStackFrame *frame);

  inline ZStackFrame* getDataFrame() {
    return m_dataFrame;
  }

  void loadStack(const QString &fileName);
  void save(const QString &fileName);

private:
  static ZJsonObject getNodeJson(const ZTreeNode<ZObject3dScan> *node);
  void loadJsonNode(ZTreeNode<ZObject3dScan> *parent,
                    const ZJsonObject &nodeJson);


signals:

public slots:

private:
  ZStack *m_stack;
  ZTree<ZObject3dScan> m_labelTree;
  ZStackFrame *m_dataFrame;
};

#endif // ZSEGMENTATIONPROJECT_H

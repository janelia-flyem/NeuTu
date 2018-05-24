#ifndef ZFLYEMSTACKDOC_H
#define ZFLYEMSTACKDOC_H

#include "tz_graph.h"
#include "zstackdoc.h"
#include "zsynapseannotationarray.h"
#include "zsuperpixelmaparray.h"
#include "zgraph.h"
#include "zsegmentationbundle.h"

#include <vector>
#include <string>
#include <map>

class ZFlyEmStackDoc : public ZStackDoc
{
  Q_OBJECT
public:
  ZFlyEmStackDoc(QObject *parent = NULL);
  virtual ~ZFlyEmStackDoc();

  virtual bool isDeprecated(EComponent component);
  virtual void deprecate(EComponent component);
  virtual void deprecateDependent(EComponent component);

  void updateBodyGraph();
  inline ZGraph* getBodyGraph() { return m_segmentationBundle.getBodyGraph(); }
  void setSuperpixelMap(const ZSuperpixelMapArray &superpixelMap);
  void appendBodyNeighbor(std::vector<std::vector<double> > *selected);

  QString rawDataInfo(double x, double y, int z, neutube::EAxis axis) const;
//  using ZStackDoc::rawDataInfo; // fix warning -Woverloaded-virtual

  //void setSegmentation(ZStack *seg);
  ZStack* getSegmentation() const;

  virtual ZStack *stackMask() const;
  virtual ZStack *getStack() const;
  virtual ZStack*& stackRef();

  void importSegmentation(const std::string &filePath);

  void updateBodyObject();
  void updateBodyIndexMap();
  ZObject3dArray* getBodyObject();
  ZObject3dArray* getBodyBoundaryObject();
  ZObject3d* getBodyObject(int id);
  ZObject3d* getBodyBoundaryObject(int id);
  ZObject3d* getBodyObjectBorder(int id1, int id2);
  std::vector<ZObject3d*> getNeighborBodyObject(int id);
  std::map<int,size_t>* getBodyIndexMap();

  inline flyem::ZSegmentationBundle* getSegmentationBundle() {
    return &m_segmentationBundle;
  }

  bool importAxonExport(const std::string &filePath);

signals:
  
public slots:

private:
  //ZStack *m_segmentation;
  //ZGraph *m_bodyGraph;
  //std::map<int, int> m_bodyIndexMap;
  //std::vector<ZObject3d> m_bodyObject;
  flyem::ZSynapseAnnotationArray synapseArray;
  //ZSuperpixelMapArray m_superpixelMap;
  mutable flyem::ZSegmentationBundle m_segmentationBundle;
};

#endif // ZFLYEMSTACKDOC_H

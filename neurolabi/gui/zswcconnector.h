#ifndef ZSWCCONNECTOR_H
#define ZSWCCONNECTOR_H

#include <utility>
#include <vector>
#include "swctreenode.h"
#include "zswctree.h"
#include "zswcpath.h"
#include "zresolution.h"

class ZGraph;

class ZSwcConnector
{
public:
  ZSwcConnector();

  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcPath &hook, const ZSwcTree &loop);
  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcPath &hook, const std::vector<ZSwcTree*> &loop);

  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcTree &hook, const ZSwcTree &loop);
  std::pair<Swc_Tree_Node*, Swc_Tree_Node*> identifyConnection(
      const ZSwcTree &hook, const std::vector<ZSwcTree*> &loop);

  inline double getConnDist() const { return m_dist; }
  inline void setConnDist(double dist) { m_dist = dist; }

  inline void setMinDist(double dist) { m_minDist = dist; }

  ZGraph* buildConnection(const std::vector<Swc_Tree_Node *> &nodeArray);
  ZGraph* buildConnection(const std::set<Swc_Tree_Node *> &nodeSet);

  void setResolution(const ZResolution &resolution);

private:
  double m_minDist;
  double m_dist;
  ZResolution m_resolution;
};

#endif // ZSWCCONNECTOR_H

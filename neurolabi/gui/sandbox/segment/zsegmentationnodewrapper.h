#ifndef ZSEGMENTATIONNODEWRAPPER_H
#define ZSEGMENTATIONNODEWRAPPER_H


#include<memory>
#include<string>
#include"zstackobject.h"
#include"zsegmentationtree.h"


using std::shared_ptr;
using std::string;


class ZSegmentationNodeWrapper: public ZStackObject{
public:
  ZSegmentationNodeWrapper(shared_ptr<ZSegmentationTree> tree, const string& node_id)
  :m_tree(tree),m_id(node_id){m_type= ZStackObject::EType::SEGMENTATION_ENCODER;}

  ~ZSegmentationNodeWrapper() override {
    /*std::cout<<"Node wrapper deleted: "<<m_id<<std::endl;*/}

public:
//  const std::string& className() const override {static std::string name("ZSegmentationNodeWrapper");return name;}
  bool display(QPainter */*painter*/, const DisplayConfig &/*config*/) const override {
    return false;
  }
  /*
  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option, neutu::EAxis sliceAxis) const override;
      */
  virtual bool hit(double x, double y, double z) override;
  string getNodeID()const{return m_id;}
  ZCuboid getBoundBox() const override;

private:
  shared_ptr<ZSegmentationTree> m_tree;
  string m_id;
};


#endif // ZSEGMENTATIONNODEWRAPPER_H

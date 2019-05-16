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
    :m_tree(tree),m_id(node_id){m_type=EType::SEGMENTATION_ENCODER;}

  ~ZSegmentationNodeWrapper(){/*std::cout<<"Node wrapper deleted: "<<m_id<<std::endl;*/}

public:
  virtual const std::string& className() const {static std::string name("ZSegmentationNodeWrapper");return name;}
  virtual void display(ZPainter &painter, int slice, EDisplayStyle option, neutu::EAxis sliceAxis) const;
  virtual bool hit(double x, double y, double z);
  string getNodeID()const{return m_id;}

private:
  shared_ptr<ZSegmentationTree> m_tree;
  string m_id;
};


#endif // ZSEGMENTATIONNODEWRAPPER_H

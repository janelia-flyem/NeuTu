#include "zsegmentationcomposite.h"

ZSegmentationComposite::ZSegmentationComposite(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent)
:ZSegmentationNode(label, encoder_factory,parent){


}


ZSegmentationComposite::~ZSegmentationComposite(){
  clear();
}


void ZSegmentationComposite::labelStack(ZStack &stack, int v)const{
  int l = (v==0) ? getLabel() : v;
  for(auto child: m_children){
    child->labelStack(stack, l);
  }
}


bool ZSegmentationComposite::contains(int x, int y, int z)const{
  for(auto child : m_children){
    bool rv = child->contains(x,y,z);
    if(rv){
      return true;
    }
  }
  return false;
}


ZIntCuboid ZSegmentationComposite::getBoundBox()const{
  ZIntCuboid box;
  for(auto child: m_children){
    box.join(child->getBoundBox());
  }
  return box;
}


void ZSegmentationComposite::removeChildByLabel(int label){
  for(auto it = m_children.begin(); it != m_children.end(); ++it){
    if((*it)->getLabel() == label){
      m_children.erase(it);
      break;
    }
  }
}


ZSegmentationNode* ZSegmentationComposite::find(const std::string &id){
  if(getID() == id){
    return this;
  } else {
    ZSegmentationNode* rv = nullptr;
    for(auto child: m_children){
      rv = child->find(id);
      if(rv){
        return rv;
      }
    }
  }
  return nullptr;
}


ZSegmentationNode* ZSegmentationComposite::getChildByLabel(int label){
  for(auto child : m_children){
    if(child->getLabel() == label){
      return child.get();
    }
  }
  return nullptr;
}


void ZSegmentationComposite::consume(const ZStack &stack){
  clear();
  const uint8_t* p = stack.array8();
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();
  int slice = width*height;
  int ofx = stack.getOffset().getX();
  int ofy = stack.getOffset().getY();
  int ofz = stack.getOffset().getZ();
  int index = 0;
  int v = 0;

  for(int d = 0; d < depth; ++d){
    for(int h = 0; h < height; ++h){
      index = d * slice + h * width;
      for(int w = 0; w < width; ++w){
        v = p[index++];
        if(v){
          ZSegmentationNode* child = getChildByLabel(v);
          if(child){
            child->add(ofx+w,ofy+h,ofz+d);
          } else {
            child = new ZSegmentationLeaf(v,m_encoder_factory,this);
            child->add(ofx+w,ofy+h,ofz+d);
            m_children.push_back(shared_ptr<ZSegmentationNode>(child));
          }
        }
      }
    }
  }
}


vector<int> ZSegmentationComposite::getChildrenLabels()const{
  vector<int> rv;
  for(auto child : m_children){
    rv.push_back(child->getLabel());
  }
  return rv;
}


double ZSegmentationComposite::memUsage()const{
  double rv = 0.0;
  for(auto child: m_children){
    rv += child->memUsage();
  }
  return rv;
}


void ZSegmentationComposite::replace(ZSegmentationNode *target, ZSegmentationNode *source){
  if(!target || !source){
    return;
  }
  source->copyID(target);
  removeChildByLabel(target->getLabel());
  source->setParent(this);
  m_children.push_back(shared_ptr<ZSegmentationNode>(source));
}

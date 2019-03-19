#include<sstream>
#include "zsegmentationnode.h"


using std::stringstream;


ZSegmentationNode::ZSegmentationNode(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent)
:m_label(label), m_encoder_factory(encoder_factory), m_parent(parent){
  m_id = getNextID();
}


string ZSegmentationNode::getNextID()const {
  static int id = -1;
  id++;
  string rv = "";
  stringstream s;
  s<<id;
  s>>rv;
  return rv;
}


void ZSegmentationNode::maskStack(ZStack &stack) const{
  ZStack* tmp = stack.clone();
  tmp->setZero();
  labelStack(tmp);
  //tmp->save("/home/deli/mask.tif");
  uint8_t* p = stack.array8();
  uint8_t* q = tmp->array8();
  uint8_t* pend = p + stack.getVoxelNumber();
  for(; p != pend; ++p, ++q){
    if(!(*q)) *p = 0;
  }
  delete tmp;
}


ZSegmentationLeaf::~ZSegmentationLeaf(){

}


ZSegmentationLeaf::ZSegmentationLeaf(int label, shared_ptr<ZSegmentationEncoderFactory> factory, ZSegmentationNode* parent)
:ZSegmentationNode(label,factory,parent){
  m_encoder = shared_ptr<ZSegmentationEncoder>(m_encoder_factory->create());
}


void ZSegmentationLeaf::add(int x, int y, int z){
  m_encoder->add(x,y,z);
}


void ZSegmentationLeaf::labelStack(ZStack &stack, int v)const{
  if(v == 0){
    m_encoder->labelStack(stack, getLabel());
  } else{
    m_encoder->labelStack(stack, v);
  }
}


void ZSegmentationLeaf::consume(const ZStack & stack){
  ZSegmentationNode* p = new ZSegmentationComposite(getLabel(),m_encoder_factory);
  p->consume(stack);
  getParent()->replace(this,p);
}


bool ZSegmentationLeaf::contains(int x, int y, int z) const{
  return m_encoder->contains(x,y,z);
}


void ZSegmentationLeaf::merge(ZSegmentationNode *node){
  ZSegmentationNode* p = this;
  while(p){
    if(p == node){
      return;
    }
    p = p->getParent();
  }

  vector<ZSegmentationNode*> leaf_nodes = node->getLeaves();
  for(auto node: leaf_nodes){
    m_encoder->unify(node->getEncoder());
  }
  ZSegmentationNode* parent = node->getParent();
  node->setParent(nullptr);
  if(parent){
    parent->removeChildByLabel(node->getLabel());
  }
}


ZSegmentationComposite::ZSegmentationComposite(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent)
:ZSegmentationNode(label, encoder_factory,parent){


}


ZSegmentationComposite::~ZSegmentationComposite(){
  clear();
}


vector<ZSegmentationNode*> ZSegmentationComposite::getLeaves(){
  vector<ZSegmentationNode*> rv;
  for(auto child: m_children){
    vector<ZSegmentationNode*> t = child->getLeaves();
    rv.insert(rv.end(),t.begin(),t.end());
  }
  return rv;
}


void ZSegmentationComposite::merge(ZSegmentationNode *node){
  if(!node){
    return;
  }
  ZSegmentationNode* p = this;
  while(p){
    if(p == node){
      return;
    }
    p = p->getParent();
  }
  ZSegmentationNode* parent = node->getParent();
  if(parent == this){
    return;
  }

  int label = 1;
  for(;;++label){
    bool flag = true;
    for(auto child: m_children){
      if(child->getLabel() == label){
        flag = false;
        break;
      }
    }
    if(flag){
      break;
    }
  }
  if(parent){
    shared_ptr<ZSegmentationNode> snode = parent->getChildByLabel(node->getLabel());
    if(snode){
      parent->removeChildByLabel(snode->getLabel());
      snode->setParent(this);
      m_children.push_back(snode);
      snode->setLabel(label);
    }
  } /*else {
    node->setParent(this);
    m_children.push_back(shared_ptr<ZSegmentationNode>(node));
    node->setLabel(label);
  }*/
}


void ZSegmentationComposite::labelStack(ZStack &stack, int v)const{
  int l = (v==0) ? getLabel() : v;
  for(auto child: m_children){
    child->labelStack(stack, l);
  }
}


bool ZSegmentationComposite::contains(int x, int y, int z)const{
  for(auto child : m_children){
    if(child && child->contains(x,y,z)){
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


shared_ptr<ZSegmentationNode> ZSegmentationComposite::getChildByLabel(int label){
  for(auto child : m_children){
    if(child->getLabel() == label){
      return child;
    }
  }
  return nullptr;
}


vector<string> ZSegmentationComposite::getAllIDs() const{
  vector<string> ids{this->getID()};
  for(auto child: m_children){
    if(child){
      vector<string> t = child->getAllIDs();
      ids.insert(ids.end(),t.begin(),t.end());
    }
  }
  return ids;
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
          shared_ptr<ZSegmentationNode> child = getChildByLabel(v);
          if(child){
            child->add(ofx+w,ofy+h,ofz+d);
          } else {
            //std::cout<<111111<<std::endl;
            child = shared_ptr<ZSegmentationNode>(new ZSegmentationLeaf(v,m_encoder_factory,this));
            child->getEncoder()->initBoundBox(stack.getBoundBox());
            child->add(ofx+w,ofy+h,ofz+d);
            m_children.push_back(child);
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


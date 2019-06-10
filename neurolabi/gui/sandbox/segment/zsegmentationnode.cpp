#include<sstream>
#include "zsegmentationnode.h"
#include "zcolorscheme.h"


using std::stringstream;


ZSegmentationNode::ZSegmentationNode(int label, ZSegmentationNode* parent)
:m_label(label), m_parent(parent){
  m_id = getNextID();
  int id;
  stringstream s;
  s<<m_id;
  s>>id;
  static ZColorScheme scheme;
  scheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
  m_color = scheme.getColor(id);
  m_color.setAlpha(50);
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


ZSegmentationLeaf::ZSegmentationLeaf(int label,const ZIntPoint& offset, ZSegmentationNode* parent)
:ZSegmentationNode(label,parent){
  m_encoder = shared_ptr<ZSegmentationEncoder>(new ZSegmentationEncoder(offset));
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
  ZSegmentationNode* p = new ZSegmentationComposite(getLabel());
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
    m_encoder->unify(node->getEncoder().get());
  }
  ZSegmentationNode* parent = node->getParent();
  node->setParent(nullptr);
  if(parent){
    parent->removeChildByLabel(node->getLabel());
  }
}


ZSegmentationComposite::ZSegmentationComposite(int label, ZSegmentationNode* parent)
:ZSegmentationNode(label,parent){

}


ZSegmentationComposite::~ZSegmentationComposite(){
  clear();
}


void ZSegmentationComposite::group(const map<int, vector<int> > &groups){
  if(!groups.size()){
    return;
  }

  for(auto it = groups.begin(); it != groups.end(); ++it){
    const vector<int>& group = it->second;
    int label = 1;
    std::vector<int> vec_labels = getChildrenLabels();
    std::set<int> set_labels(vec_labels.begin(),vec_labels.end());
    for(;;++label){//find first not used label
      if(set_labels.find(label) == set_labels.end()){
        break;
      }
    }

    shared_ptr<ZSegmentationComposite> p = shared_ptr<ZSegmentationComposite>(new ZSegmentationComposite(label,this));

    for(auto child_label: group){
      shared_ptr<ZSegmentationNode> child = getChildByLabel(child_label);
      removeChildByLabel(child_label);
      p->m_children.push_back(child);
      child->setParent(p.get());
    }
    if(p->getChildrenLabels().size()){
      m_children.push_back(p);
    }
  }
}


vector<ZSegmentationNode*> ZSegmentationComposite::getLeaves(){
  vector<ZSegmentationNode*> rv;
  for(auto child: m_children){
    vector<ZSegmentationNode*> t = child->getLeaves();
    rv.insert(rv.end(),t.begin(),t.end());
  }
  return rv;
}


shared_ptr<ZSegmentationEncoder> ZSegmentationComposite::getEncoder(){
  //create dymatically
  if(m_children.size() == 0){
    return nullptr;
  }
  shared_ptr<ZSegmentationEncoder> rv = (*m_children.begin())->getEncoder();
  if(rv){
    rv = shared_ptr<ZSegmentationEncoder>(rv->clone());//make sure not to change the original one
    auto it = m_children.begin();
    for(++it; it != m_children.end(); ++it){
      rv->unify((*it)->getEncoder().get());
    }
    return rv;
  }
  return nullptr;
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


template<typename T>
void ZSegmentationComposite::_consume(const T* array, const ZStack& stack){
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
      int prev = 0;
      int x1 = 0;
      int x2 = 0;
      for(int w = 0; w < width; ++w){
        v = array[index++];
        if(v == prev){
          ++x2;
        } else {
          if(prev){
            shared_ptr<ZSegmentationNode> child = getChildByLabel(prev);
            if(child){
              child->getEncoder()->addSegment(ofz+d,ofy+h,ofx+x1,ofx+x2);
            } else {
              child = shared_ptr<ZSegmentationNode>(new ZSegmentationLeaf(prev,stack.getOffset(),this));
              child->getEncoder()->addSegment(ofz+d,ofy+h,ofx+x1,ofx+x2);
              m_children.push_back(child);
            }
          }
          x1 = x2 = w;
          prev = v;
        }
      }
      if(prev){
        shared_ptr<ZSegmentationNode> child = getChildByLabel(prev);
        if(child){
          child->getEncoder()->addSegment(ofz+d,ofy+h,ofx+x1,ofx+x2);
        } else {
          child = shared_ptr<ZSegmentationNode>(new ZSegmentationLeaf(prev,stack.getOffset(),this));
          child->getEncoder()->addSegment(ofz+d,ofy+h,ofx+x1,ofx+x2);
          m_children.push_back(child);
        }
      }
    }
  }
}


void ZSegmentationComposite::consume(const ZStack &stack){
  clear();
  if(stack.kind() == GREY){
    _consume<uint8>(stack.array8(),stack);
  } else if(stack.kind() == GREY16){
    _consume<uint16>(stack.array16(),stack);
  } else if(stack.kind() == FLOAT32){
    _consume<float>(stack.array32(),stack);
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


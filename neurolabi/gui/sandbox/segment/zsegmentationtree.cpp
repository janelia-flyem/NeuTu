#include<sstream>
#include"zsegmentationtree.h"
#include"zstack.hxx"
#include"zintcuboid.h"
#include"zpainter.h"

using std::stringstream;


ZSegmentationTree::~ZSegmentationTree(){

}


ZSegmentationTree::ZSegmentationTree(ZSegmentationEncoderFactory* encoder_factory){

  m_encoder_factory = shared_ptr<ZSegmentationEncoderFactory>(encoder_factory);
  m_root = shared_ptr<ZSegmentationNode>(new ZSegmentationComposite(0,m_encoder_factory,nullptr));
}


void ZSegmentationTree::consume(const std::string &id, const ZStack &stack){
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    node->consume(stack);
    notify(id);
  }
}


ZIntCuboid ZSegmentationTree::getBoundBox(const std::string &id) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->getBoundBox();
  }
  return ZIntCuboid(0,0,0,-1,-1,-1);
}


void ZSegmentationTree::maskStack(const std::string &id, ZStack &stack) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    node->maskStack(stack);
  }
}


vector<int> ZSegmentationTree::getChildrenLabels(const string& id) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->getChildrenLabels();
  }
  return vector<int>();
}


vector<string> ZSegmentationTree::getChildrenIDs(const std::string &id) const{
  ZSegmentationNode* node = m_root->find(id);
  vector<string> rv;
  if(node){
    for(auto label: node->getChildrenLabels()){
      auto child = node->getChildByLabel(label);
      if(child){
        rv.push_back(child->getID());
      }
    }
  }
  return rv;
}


bool ZSegmentationTree::hasID(const std::string &id) const{
  vector<string> ids = m_root->getAllIDs();
  for(auto i: ids){
    if(i == id){
      return true;
    }
  }
  return false;
}


string ZSegmentationTree::getChildID(const std::string &id, int label) const{
  ZSegmentationNode* parent = m_root->find(id);
  if(parent){
    auto child = parent->getChildByLabel(label);
    if(child){
      return child->getID();
    }
  }
  return "";
}


void ZSegmentationTree::labelStack(const std::string &id, ZStack &stack, int label) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    node->labelStack(stack,label);
  }
}


void ZSegmentationTree::removeObserver(ZSegmentationTreeObserver *observer){
  for(auto it = m_observers.begin(); it != m_observers.end(); ++it){
    if(*it == observer){
      m_observers.erase(it);
      break;
    }
  }
}


void ZSegmentationTree::notify(const std::string &id)const{
  for(ZSegmentationTreeObserver* ob: m_observers){
    ob->update(this,id);
  }
}


void ZSegmentationTree::clear(){
  m_root->clear();
}


ZSegmentationEncoder* ZSegmentationTree::getEncoder(const std::string &id){
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->getEncoder();
  }
  return nullptr;
}


double ZSegmentationTree::memUsage()const{
  return m_root->memUsage();
}


bool ZSegmentationTree::isLeaf(const std::string &id) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->isLeaf();
  }
  return false;
}


void ZSegmentationTree::merge(const std::string &from_id, const std::string &to_id){
  ZSegmentationNode* from = m_root->find(from_id);
  ZSegmentationNode* to = m_root->find(to_id);

  if(!from || !to || from == to){
    return;
  }
  ZSegmentationNode* from_parent = from->getParent();
  to->merge(from);
  if(from_parent){
    notify(from_parent->getID());
  }
  notify(to->getID());
}


bool ZSegmentationTree::contains(const std::string &id, int x, int y, int z) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->contains(x,y,z);
  }
  return false;
}


vector<string> ZSegmentationTree::getLeavesIDs(const std::string &id) const{
  ZSegmentationNode* node = m_root->find(id);
  vector<string> rv;
  if (node){
    for(auto leaf: node->getLeaves()){
      rv.push_back(leaf->getID());
    }
  }
  return rv;
}


vector<string> ZSegmentationTree::getAllIDs(const std::string &id) const{
  ZSegmentationNode* node = m_root->find(id);
  if(node){
    return node->getAllIDs();
  }
  return vector<string>();
}

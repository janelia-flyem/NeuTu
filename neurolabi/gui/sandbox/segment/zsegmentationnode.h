#ifndef ZSEGMENTATIONNODE_H
#define ZSEGMENTATIONNODE_H


#include<memory>
#include"zsegmentationencoder.h"


using std::shared_ptr;


class ZSegmentationNode{
public:
  ZSegmentationNode(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationNode(){}

protected:
  ZSegmentationNode(){}

public:
  inline int getLabel()const{return  m_label;}
  void setLabel(int label){m_label = label;}
  string getID()const{return m_id;}

public:
  virtual void consume(const ZStack& stack) = 0;
  void consume(const ZStack* pstack){if(pstack)consume(*pstack);}

  virtual shared_ptr<ZSegmentationNode> getChildByLabel(int label) = 0;

  virtual void add(int x, int y, int z) = 0;
  inline void add(const ZIntPoint& point){add(point.getX(),point.getY(),point.getZ());}

  virtual bool isLeaf()const = 0;

  virtual vector<int> getChildrenLabels()const = 0;

  virtual void removeChildByLabel(int label) = 0;

  virtual ZSegmentationNode* find(const string& id) = 0;

  virtual ZIntCuboid getBoundBox()const = 0;

  virtual double memUsage()const = 0;

  virtual void labelStack(ZStack& stack, int v = 0) const = 0;
  void labelStack(ZStack* pstack)const{if(pstack)labelStack(*pstack);}

  void maskStack(ZStack& stack)const;
  void maskStack(ZStack* pstack)const{if(pstack){maskStack(*pstack);}}

  ZSegmentationNode* getParent(){return m_parent;}

  void setParent(ZSegmentationNode* parent){m_parent = parent;}

  virtual void replace(ZSegmentationNode* target, ZSegmentationNode* source) = 0;

  void copyID(ZSegmentationNode* node){m_id = node->m_id;}

  virtual void clear() = 0;

  virtual ZSegmentationEncoder* getEncoder() = 0;

  virtual bool contains(int x, int y, int z) const =0;

  bool contains(const ZIntPoint& p)const{return contains(p.getX(),p.getY(),p.getZ());}

  virtual void merge(ZSegmentationNode* node) = 0;

  virtual vector<ZSegmentationNode*> getLeaves() = 0;

  virtual vector<string> getAllIDs()const=0;

protected:
  string getNextID()const;

protected:
  int m_label;//segmentation label
  string m_id;//universal id
  shared_ptr<ZSegmentationEncoderFactory> m_encoder_factory;
  ZSegmentationNode* m_parent;
};


class ZSegmentationLeaf: public ZSegmentationNode{
public:
  ZSegmentationLeaf(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationLeaf();

private:
  ZSegmentationLeaf(){}

public:
  virtual void add(int x, int y, int z);

  virtual bool isLeaf()const {return true;}

  virtual ZSegmentationNode* find(const string& id){if (id == getID()) return this;return nullptr;}

  virtual ZIntCuboid getBoundBox()const {return m_encoder->getBoundBox();}
  //virtual void unify(const ZSegmentationNode& node);

  virtual double memUsage()const{return m_encoder->memUsage();}

  virtual void labelStack(ZStack& stack, int v = 0) const;

  virtual void consume(const ZStack&);

  virtual void clear(){}

  virtual ZSegmentationEncoder* getEncoder(){return m_encoder.get();}

  virtual bool contains(int x, int y, int z) const;

  virtual void merge(ZSegmentationNode* node);

  virtual vector<ZSegmentationNode*> getLeaves(){vector<ZSegmentationNode*>rv{this};return rv;}

  virtual vector<string> getAllIDs()const{vector<string> rv{this->getID()}; return rv;}

public://should never be called
  virtual shared_ptr<ZSegmentationNode> getChildByLabel(int){return nullptr;}
  virtual vector<int> getChildrenLabels()const{return vector<int>();}
  virtual void removeChildByLabel(int){}
  virtual void replace(ZSegmentationNode*, ZSegmentationNode*){}

private:
  shared_ptr<ZSegmentationEncoder> m_encoder;
};


class ZSegmentationComposite: public ZSegmentationNode{
public:
  ZSegmentationComposite(int label, shared_ptr<ZSegmentationEncoderFactory> encoder_factory, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationComposite();

private:
  ZSegmentationComposite(){}

public:
  virtual void consume(const ZStack& stack);

  virtual shared_ptr<ZSegmentationNode> getChildByLabel(int label);

  virtual bool isLeaf()const {return false;}

  virtual vector<int> getChildrenLabels()const;

  virtual ZSegmentationNode* find(const string& id);

  virtual ZIntCuboid getBoundBox()const;

  virtual double memUsage()const;

  virtual void labelStack(ZStack& stack, int v = 0) const;

  virtual void removeChildByLabel(int label);

  virtual void replace(ZSegmentationNode* target, ZSegmentationNode* source);

  virtual void clear(){m_children.clear();}

  virtual bool contains(int x, int y, int z)const;

  virtual void merge(ZSegmentationNode* node);

  virtual vector<ZSegmentationNode*> getLeaves();

  virtual vector<string> getAllIDs()const;

public://should never be called
  virtual void add(int, int, int){}
  virtual ZSegmentationEncoder* getEncoder(){return nullptr;}

private:
  vector<shared_ptr<ZSegmentationNode>> m_children;
};


#endif // ZSEGMENTATIONNODE_H

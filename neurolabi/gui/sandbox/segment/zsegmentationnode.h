#ifndef ZSEGMENTATIONNODE_H
#define ZSEGMENTATIONNODE_H


#include<memory>
#include<string>
#include<map>
#include"zsegmentationencoder.h"


using std::shared_ptr;
using std::map;
//using std::string;


class ZSegmentationNode{
public:
  ZSegmentationNode(int label, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationNode(){}

protected:
  ZSegmentationNode(){}

public:
  inline int getLabel()const{return  m_label;}
  void setLabel(int label){m_label = label;}
  std::string getID()const{return m_id;}
  void setColor(const QColor& color){m_color = color;}
  QColor getColor()const{return m_color;}

public:
  virtual void consume(const ZStack& stack) = 0;
  void consume(const ZStack* pstack){if(pstack)consume(*pstack);}

  virtual shared_ptr<ZSegmentationNode> getChildByLabel(int label) = 0;

  virtual void add(int x, int y, int z) = 0;
  inline void add(const ZIntPoint& point){add(point.getX(),point.getY(),point.getZ());}

  virtual bool isLeaf()const = 0;

  virtual vector<int> getChildrenLabels()const = 0;

  virtual void removeChildByLabel(int label) = 0;

  virtual ZSegmentationNode* find(const std::string& id) = 0;

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

  virtual shared_ptr<ZSegmentationEncoder> getEncoder() = 0;

  virtual bool contains(int x, int y, int z) const =0;

  bool contains(const ZIntPoint& p)const{return contains(p.getX(),p.getY(),p.getZ());}

  virtual void merge(ZSegmentationNode* node) = 0;

  virtual vector<ZSegmentationNode*> getLeaves() = 0;

  virtual vector<std::string> getAllIDs()const=0;
  virtual bool hasId(const std::string &id) const = 0;

  virtual void group(const map<int,vector<int>>& groups)=0;

  //virtual map<string,map<string,int>> getAdjMatrix()const=0;

  //virtual int getVoxelNumber()const=0;

protected:
  std::string getNextID()const;

protected:
  int m_label;//segmentation label
  std::string m_id;//universal id
  ZSegmentationNode* m_parent;
  QColor m_color;
};


class ZSegmentationLeaf: public ZSegmentationNode{
public:
  ZSegmentationLeaf(int label, const ZIntPoint& offset, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationLeaf();

private:
  ZSegmentationLeaf(){}

public:
  void add(int x, int y, int z) override;

  bool isLeaf() const override {return true;}

  ZSegmentationNode* find(const std::string& id) override {
    if (id == getID()) return this;
    return nullptr;
  }

  ZIntCuboid getBoundBox() const override {return m_encoder->getBoundBox();}
  //virtual void unify(const ZSegmentationNode& node);

  double memUsage() const override {return m_encoder->memUsage();}

  void labelStack(ZStack& stack, int v = 0) const override;

  void consume(const ZStack&) override;

  void clear() override {}

  shared_ptr<ZSegmentationEncoder> getEncoder() override {return m_encoder;}

  bool contains(int x, int y, int z) const override;

  void merge(ZSegmentationNode* node) override;

  vector<ZSegmentationNode*> getLeaves() override {
    vector<ZSegmentationNode*>rv{this};return rv;
  }

  vector<std::string> getAllIDs() const override {
    vector<std::string> rv{this->getID()}; return rv;
  }

  bool hasId(const std::string &id) const override {
    return this->getID() == id;
  }

  //virtual map<string,map<string,int>> getAdjMatrix()const;

  //virtual int getVoxelNumber()const;

public://should never be called

  shared_ptr<ZSegmentationNode> getChildByLabel(int) override {
    return nullptr;
  }
  vector<int> getChildrenLabels()const override{
    return vector<int>();
  }
  void removeChildByLabel(int) override {}
  void replace(ZSegmentationNode*, ZSegmentationNode*) override {}
  void group(const map<int,vector<int>>&) override {}

private:
  shared_ptr<ZSegmentationEncoder> m_encoder;
};


class ZSegmentationComposite: public ZSegmentationNode{
public:
  ZSegmentationComposite(int label, ZSegmentationNode* parent = nullptr);
  virtual ~ZSegmentationComposite();

private:
  ZSegmentationComposite(){}

public:
  void consume(const ZStack& stack) override;

  shared_ptr<ZSegmentationNode> getChildByLabel(int label) override;

  bool isLeaf() const override {return false;}

  vector<int> getChildrenLabels() const override;

  ZSegmentationNode* find(const std::string& id) override;

  ZIntCuboid getBoundBox()const override;

  double memUsage()const override;

  void labelStack(ZStack& stack, int v = 0) const override;

  void removeChildByLabel(int label) override;

  void replace(ZSegmentationNode* target, ZSegmentationNode* source) override;

  void clear() override {m_children.clear();}

  bool contains(int x, int y, int z)const override;

  void merge(ZSegmentationNode* node) override;

  vector<ZSegmentationNode*> getLeaves() override;

  vector<std::string> getAllIDs() const override;
  bool hasId(const std::string &id) const override;

  shared_ptr<ZSegmentationEncoder> getEncoder() override;

  void group(const map<int,vector<int>>& groups) override;
  //virtual map<string,map<string,int>> getAdjMatrix()const;

  //virtual int getVoxelNumber()const;

public://should never be called
  virtual void add(int, int, int){}

private:
  template<typename T>
  void _consume(const T* array, const ZStack& stack);

private:
  vector<shared_ptr<ZSegmentationNode>> m_children;
};


#endif // ZSEGMENTATIONNODE_H

#ifndef ZSEGMENTATIONTREE_H
#define ZSEGMENTATIONTREE_H


#include<memory>
#include<vector>
#include<map>
#include"zsegmentationencoder.h"
#include"zsegmentationnode.h"


using std::shared_ptr;
using std::vector;
using std::map;


class ZSegmentationTreeObserver;


class ZSegmentationTree{
public:
  ZSegmentationTree();
  ~ZSegmentationTree();

public:
  std::string getRootID()const{return m_root->getID();}

  void consume(const std::string& id, const ZStack& stack);
  void consume(const std::string& id, const ZStack* pstack){if(pstack)consume(id,*pstack);}

  ZIntCuboid getBoundBox(const std::string& id)const;

  void maskStack(const std::string& id, ZStack& stack)const;
  void maskStack(const std::string& id, ZStack* pstack)const{if(pstack)maskStack(id,*pstack);}

  vector<int> getChildrenLabels(const std::string& id)const;

  std::string getChildID(const std::string& id, int label)const;
  vector<std::string> getChildrenIDs(const std::string& id)const;

  vector<std::string> getLeavesIDs(const std::string& id)const;

  vector<std::string> getAllIDs(const std::string& id)const;

  void labelStack(const std::string& id, ZStack& stack, int label = 0)const;
  void labelStack(const std::string& id, ZStack* pstack, int label = 0)const{if(pstack)labelStack(id,*pstack,label);}

  void addObserver(ZSegmentationTreeObserver* observer){m_observers.push_back(observer);}
  void removeObserver(ZSegmentationTreeObserver* observer);

  void clear();

  void merge(const std::string& from_id, const std::string& to_id);

  double memUsage()const;

  shared_ptr<ZSegmentationEncoder> getEncoder(const std::string& id);

  bool isLeaf(const std::string& id)const;

  bool contains(const std::string& id, int x, int y, int z)const;

  bool hasID(const std::string& id)const;

  int getLabel(const std::string& id) const;

  void group(const std::string& id, map<int,vector<int>>& groups);

  QColor getColor(const std::string& id)const;

private:
  void notify(const std::string& id)const;

private:
  shared_ptr<ZSegmentationNode> m_root;
  vector<ZSegmentationTreeObserver*> m_observers;
};


class ZSegmentationTreeObserver{
public:
  ZSegmentationTreeObserver(){}
  virtual ~ZSegmentationTreeObserver(){}

public:
  virtual void update(const ZSegmentationTree* m_tree, const std::string& id) = 0;

protected:

};


#endif // ZSEGMENTATIONTREE_H

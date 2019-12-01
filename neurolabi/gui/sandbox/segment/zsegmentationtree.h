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
  string getRootID()const{return m_root->getID();}

  void consume(const string& id, const ZStack& stack);
  void consume(const string& id, const ZStack* pstack){if(pstack)consume(id,*pstack);}

  ZIntCuboid getBoundBox(const string& id)const;

  void maskStack(const string& id, ZStack& stack)const;
  void maskStack(const string& id, ZStack* pstack)const{if(pstack)maskStack(id,*pstack);}

  vector<int> getChildrenLabels(const string& id)const;

  string getChildID(const string& id, int label)const;
  vector<string> getChildrenIDs(const string& id)const;

  vector<string> getLeavesIDs(const string& id)const;

  vector<string> getAllIDs(const string& id)const;

  void labelStack(const string& id, ZStack& stack, int label = 0)const;
  void labelStack(const string& id, ZStack* pstack, int label = 0)const{if(pstack)labelStack(id,*pstack,label);}

  void addObserver(ZSegmentationTreeObserver* observer){m_observers.push_back(observer);}
  void removeObserver(ZSegmentationTreeObserver* observer);

  void clear();

  void merge(const string& from_id, const string& to_id);

  void merge(const vector<string>& from_ids, const string& to_id);

  double memUsage()const;

  shared_ptr<ZSegmentationEncoder> getEncoder(const string& id);

  bool isLeaf(const string& id)const;

  bool contains(const string& id, int x, int y, int z)const;

  bool hasID(const string& id)const;

  int getLabel(const string& id) const;

  void group(const string& id, map<int,vector<int>>& groups);

  QColor getColor(const string& id)const;

private:
  void notify(const string& id)const;

private:
  shared_ptr<ZSegmentationNode> m_root;
  vector<ZSegmentationTreeObserver*> m_observers;
};


class ZSegmentationTreeObserver{
public:
  ZSegmentationTreeObserver(){}
  virtual ~ZSegmentationTreeObserver(){}

public:
  virtual void update(const ZSegmentationTree* m_tree, const string& id) = 0;

protected:

};


#endif // ZSEGMENTATIONTREE_H

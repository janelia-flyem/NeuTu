#ifndef ZSEGMENTATION_H
#define ZSEGMENTATION_H

#include<unordered_map>
#include<vector>
#include<string>
#include "zstack.hxx"
#include"zintpoint.h"

using std::unordered_map;
using std::vector;
using std::string;

class ZStack;


//ZSegmentationRepresentation: representation & management of segmentation
template<typename T>
class ZSegmentationRepresentation{
public:
  ZSegmentationRepresentation();
  ~ZSegmentationRepresentation(){clear();}
public:

  void consume(const ZStack& stack);
  void consume(const ZStack* pstack){if(pstack)consume(*pstack);}


  void update(const ZStack& stack);
  void update(const ZStack* pstack){if(pstack)update(*pstack);}

  inline int getLabel(const ZIntPoint& point)const;
  inline int getLabel(int x, int y, int z)const{return getLabel(ZIntPoint(x,y,z));}

  vector<int> getLabels()const;
  //user should be responsible for deleting returned segmentation
  ZSegmentationRepresentation* getSegmentationByLabel(int label)const;
  ZSegmentationRepresentation* getSegmentationByLabel(vector<int>labels)const;


  inline void setLabel(const ZIntPoint& point, int value);
  inline void setLabel(int x, int y, int z, int value){setLabel(ZIntPoint(x,y,z),value);}

  void mergeLabels(int target, int source);
  void mergeLabels(int target, const vector<int>& sources){for(int s:sources)mergeLabels(target,s); }

  void maskStack(ZStack& stack)const;
  void maskStack(ZStack& stack, int label)const;
  void maskStack(ZStack& stack, const vector<int>& labels)const;
  void maskStack(ZStack* pstack)const{if(pstack){maskStack(*pstack);}}
  void maskStack(ZStack* pstack, int label)const{if(pstack)maskStack(*pstack,label);}
  void maskStack(ZStack* pstack, const vector<int>& labels)const{if(pstack)maskStack(*pstack,labels);}

  void labelStack(ZStack& stack)const;
  void labelStack(ZStack& stack, int label)const;
  void labelStack(ZStack& stack, const vector<int>& labels)const;
  void labelStack(ZStack* pstack)const{if(pstack)labelStack(*pstack);}
  void labelStack(ZStack* pstack, int label)const{if(pstack)labelStack(*pstack,label);}
  void labelStack(ZStack* pstack, const vector<int>& labels)const{if(pstack)labelStack(*pstack,labels);}

  double memUsage()const;

  void clear();

private:
  typedef T Encoder_type;
  typedef unordered_map<int,Encoder_type*> data_type;
  data_type m_data;
};


class ZSegmentationEncoder{
public:
  ZSegmentationEncoder();
  virtual ~ZSegmentationEncoder(){}
public:
  virtual void add(const ZIntPoint& point) = 0;
  void add(int x,int y,int z){add(ZIntPoint(x,y,z));}

  void add(const vector<ZIntPoint>& points){for(const ZIntPoint& p:points)add(p);}
  void add(const vector<int>& xs, const vector<int>& ys, const vector<int>& zs);

  virtual void remove(const ZIntPoint& point) = 0;
  inline void remove(int x, int y, int z){remove(ZIntPoint(x,y,z));}

  void remove (const vector<ZIntPoint>& points){for(const ZIntPoint& p:points)remove(p);}
  void remove(const vector<int>& xs, const vector<int>& ys, const vector<int>& zs);

  virtual bool contains(const ZIntPoint& point)const = 0;
  bool contains(int x, int y, int z)const{return contains(ZIntPoint(x,y,z));}

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder) = 0;
  void unify(const ZSegmentationEncoder* psegmentation_Encoder){if(psegmentation_Encoder)unify(*psegmentation_Encoder);}

  virtual void labelStack(ZStack& stack, int value)const = 0;
  void labelStack(ZStack* pstack, int value){if(pstack)labelStack(*pstack,value);}

  virtual ZSegmentationEncoder* clone()const = 0;

  virtual string type()const{return "";}

  virtual double memUsage()const=0;

  virtual void clear() = 0;

};


class ZSegmentationEncoderRLX: public ZSegmentationEncoder{
public:
  ZSegmentationEncoderRLX();
  virtual ~ZSegmentationEncoderRLX(){clear();}
public:
  virtual void add(const ZIntPoint& point);
  virtual void remove(const ZIntPoint& point);
  virtual bool contains(const ZIntPoint& point)const;
  virtual void unify(const ZSegmentationEncoder& segmentation_encoder);
  virtual void labelStack(ZStack& stack, int value)const;
  ZSegmentationEncoder* clone()const;
  virtual string type()const{return "RLX";}
  virtual double memUsage()const;
  virtual void clear();

private:
  void _addSegment(int z, int y, int start, int end);
private:
  typedef unordered_map<int,unordered_map<int,vector<int>>> data_type;
  typedef unordered_map<int,vector<int>> slice_type;
  typedef vector<int> Encoder_type;
  data_type m_data;
};


template<typename T>
ZSegmentationRepresentation<T>::ZSegmentationRepresentation(){

}


template<typename T>
void ZSegmentationRepresentation<T>::clear(){
  for(typename data_type::iterator it = m_data.begin(); it != m_data.end(); ++it){
    if(it->second){
      delete it->second;
      it->second = nullptr;
    }
  }
  m_data.clear();
}


template<typename T>
double ZSegmentationRepresentation<T>::memUsage()const{
  double rv = 0;
    for(typename data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    rv += 8;
    if(it->second){
      rv += it->second->memUsage();
    }
  }
  return rv;
}


template<typename T>
vector<int> ZSegmentationRepresentation<T>::getLabels()const{
  vector<int> rv;
  for(typename data_type::const_iterator it = m_data.begin(); it !=m_data.end(); ++it){
    rv.push_back(it->first);
  }
  return rv;
}


template<typename T>
void ZSegmentationRepresentation<T>::consume(const ZStack &stack){
  clear();

  const uint8_t* p = stack.array8();
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();
  int slice = width*height;

  typename data_type::iterator it;
  T* tmp = nullptr;
  int index = 0;
  int v = 0;

  for(int d = 0; d < depth; ++d){
    for(int h = 0; h < height; ++h){
      index = d * slice + h * width;
      for(int w = 0; w < width; ++w){
        v = p[index++];
        if(v){
          it = m_data.find(v);
          if(it != m_data.end() && it->second){
            ((ZSegmentationEncoder*)it->second)->add(w,h,d);
          } else {
            tmp = new T();
            ((ZSegmentationEncoder*)tmp)->add(w,h,d);
            m_data[v] = tmp;
          }
        }
      }
    }
  }
}


template<typename T>
int ZSegmentationRepresentation<T>::getLabel(const ZIntPoint &point) const{
  for(typename data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    if(it->second && it->second->contains(point)){
      return it->first;
    }
  }
  return 0;
}


template<typename T>
ZSegmentationRepresentation<T>* ZSegmentationRepresentation<T>::getSegmentationByLabel(int label) const{
  typename data_type::iterator it = m_data.find(label);
  if(it == m_data.end() || !it->second){
    return nullptr;
  }
  ZSegmentationRepresentation<T>* rv = new ZSegmentationRepresentation<T>();
  rv->m_data[label] = it->second->clone();
  return rv;
}


template<typename T>
ZSegmentationRepresentation<T>* ZSegmentationRepresentation<T>::getSegmentationByLabel(vector<int> labels) const{
  ZSegmentationRepresentation<T> *rv = new ZSegmentationRepresentation<T>();
  for(int label : labels){
    typename data_type::iterator it = m_data.find(label);
    if(it != m_data.end() && it->second){
      rv->m_data[label] = it->second->clone();
    }
  }
  return rv;
}


template<typename T>
void ZSegmentationRepresentation<T>::labelStack(ZStack &stack) const{
  for(typename data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    if(it->second){
      it->second->labelStack(stack, it->first);
    }
  }
}


template<typename T>
void ZSegmentationRepresentation<T>::labelStack(ZStack &stack, int label) const{
  for(typename data_type::const_iterator it = m_data.begin(); it != m_data.end(); ++it){
    if(it->first == label && it->second){
      it->second->labelStack(stack, it->first);
      break;
    }
  }
}


template<typename T>
void ZSegmentationRepresentation<T>::labelStack(ZStack &stack, const vector<int> &labels) const{
  for(int label: labels){
    labelStack(stack,label);
  }
}


template<typename T>
void ZSegmentationRepresentation<T>::maskStack(ZStack &stack) const{
  vector<int> empty_vec;
  maskStack(stack,empty_vec);
}


template<typename T>
void ZSegmentationRepresentation<T>::maskStack(ZStack &stack, int label) const{
  vector<int> labels{label};
  maskStack(stack,labels);
}


template<typename T>
void ZSegmentationRepresentation<T>::maskStack(ZStack &stack, const vector<int> &labels) const{
  ZStack* tmp = new ZStack(stack.kind(), stack.width(), stack.height(), stack.depth(), stack.channelNumber());
  //tmp->setOffset(stack.getOffset());
  if(labels.size() == 0){
    labelStack(tmp);
  } else {
    labelStack(tmp, labels);
  }
  uint8_t* p = stack.array8();
  uint8_t* q = tmp->array8();
  uint8_t* pend = p + stack.getVoxelNumber();
  for(; p != pend; ++p, ++q){
    if(!(*q)) *p = 0;
  }
  delete tmp;
}


template<typename T>
void ZSegmentationRepresentation<T>::mergeLabels(int target, int source){
  typename data_type::iterator its = m_data.find(source);
  if(its == m_data.end() || !its->second){
    return;
  }

  typename data_type::iterator itt = m_data.find(target);
  if(itt == m_data.end() || !itt->second){
    return;
  }

  ((ZSegmentationEncoder*)itt->second)->unify(its->second);

  delete its->second;
  m_data.erase(its);//
}


template<typename T>
void ZSegmentationRepresentation<T>::setLabel(const ZIntPoint &point, int value){
  for(typename data_type::iterator it = m_data.begin(); it != m_data.end(); ++it){
    if(it->second && it->second->contains(point)){
      it->second->remove(point);
      break;
    }
  }
  typename data_type::iterator it = m_data.find(value);
  if(it != m_data.end() && it->second){
    it->second->add(point);
  } else {
    T* p = new T();
    p->add(point);
    m_data[value] = p;
  }
}

template<typename T>
void ZSegmentationRepresentation<T>::update(const ZStack &stack){
  uint8_t *p = stack.array8();
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();
  int slice = width * height;

  int index = 0;
  for(int d = 0; d < depth; ++d){
    for(int h = 0; h < height; ++h){
      index = d * slice + h * width;
      for(int w = 0; w < width; ++w, ++index){
        if(p[index]){
          setLabel(w,h,d,p[index]);
        }
      }
    }
  }
}


typedef ZSegmentationRepresentation<ZSegmentationEncoderRLX> ZSegmentationRepresentationDefault;
#endif // ZSEGMENTATION_H

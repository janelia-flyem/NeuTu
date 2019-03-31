#ifndef ZSEGMENTATIONENCODER_H
#define ZSEGMENTATIONENCODER_H


#include<string>
#include<vector>
#include<memory>
#include<tuple>
#include<map>
#include<list>
#include<unordered_map>
#include"zstack.hxx"
#include"zintcuboid.h"
#include"zobject3dscan.h"


using std::string;
using std::vector;
using std::unordered_map;
using std::tuple;
using std::map;
using std::list;
using std::shared_ptr;
using std::make_tuple;


class ZSegmentationEncoder{
public:
  ZSegmentationEncoder();
  virtual ~ZSegmentationEncoder(){}

public://interfaces derived classes should override
  virtual void add(const ZIntPoint& point) = 0;

  //virtual void remove(const ZIntPoint& point) = 0;

  virtual bool contains(const ZIntPoint& point)const = 0;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder) = 0;

  virtual void labelStack(ZStack& stack, int value)const = 0;

  virtual ZSegmentationEncoder* clone()const = 0;

  virtual string type()const = 0;

  virtual double memUsage()const = 0;

  virtual ZIntCuboid getBoundBox()const = 0;

  //virtual int getVoxelNumber() const = 0;
  virtual void consume(const ZStack& /*stack*/){}

  void initBoundBox(const ZIntCuboid& box);


public:

  void add(int x,int y,int z){add(ZIntPoint(x,y,z));}

  void add(const vector<ZIntPoint>& points){for(const ZIntPoint& p:points)add(p);}

  void add(const vector<int>& xs, const vector<int>& ys, const vector<int>& zs);

  bool contains(int x, int y, int z)const{return contains(ZIntPoint(x,y,z));}

  void unify(const ZSegmentationEncoder* psegmentation_Encoder){if(psegmentation_Encoder)unify(*psegmentation_Encoder);}

  void labelStack(ZStack* pstack, int value){if(pstack)labelStack(*pstack,value);}

protected:
  ZIntCuboid m_init_box;
};


class ZSegmentationEncoderRLX: public ZSegmentationEncoder{
public:
  ZSegmentationEncoderRLX();
  virtual ~ZSegmentationEncoderRLX(){}

public:

  virtual vector<int>& getSegment(int z, int y) = 0;

  virtual const vector<int>& getSegment(int z, int y)const=0;

  virtual bool hasSegment(int z, int y)const =0;

  virtual void add(const ZIntPoint& point);

  //virtual void remove(const ZIntPoint& point);
  virtual bool contains(const ZIntPoint& point)const;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder);

  virtual void labelStack(ZStack& stack, int value)const;

  virtual ZSegmentationEncoder* clone()const = 0;

  virtual string type()const{return "RLX";}

  virtual double memUsage()const = 0;

  //void clear();
  virtual void consume(const ZStack& stack);

  virtual ZIntCuboid getBoundBox()const{return ZIntCuboid(m_minx,m_miny,m_minz,m_maxx,m_maxy,m_maxz);}

protected:

  void _addSegment(int z, int y, int start, int end);
  inline void _maybe_update_bound_box(int z, int y, int start, int end);

protected:
  int m_minx, m_maxx, m_miny, m_maxy, m_minz, m_maxz;
};


class ZSegmentationEncoderRLXTwoStageHashing:public ZSegmentationEncoderRLX{
public:
  ZSegmentationEncoderRLXTwoStageHashing(){}
  virtual ~ZSegmentationEncoderRLXTwoStageHashing(){m_data.clear();}

public:
  virtual vector<int>& getSegment(int z, int y);
  virtual const vector<int>& getSegment(int z, int y)const;
  virtual bool hasSegment(int z, int y)const;
  virtual ZSegmentationEncoder* clone()const;
  virtual double memUsage()const;

private:
  unordered_map<int,unordered_map<int,vector<int>>> m_data;
};



class ZSegmentationEncoderRLXOneStageHashing:public ZSegmentationEncoderRLX{
public:
  ZSegmentationEncoderRLXOneStageHashing(){}
  virtual ~ZSegmentationEncoderRLXOneStageHashing(){m_data.clear();}

public:
  virtual vector<int>& getSegment(int z, int y);
  virtual const vector<int>& getSegment(int z, int y)const;
  virtual bool hasSegment(int z, int y)const;
  virtual ZSegmentationEncoder* clone()const;
  virtual double memUsage()const;

public:
  class tuple_hash{
  public:
    size_t operator ()(const tuple<int,int>& key)const{
      return std::get<0>(key)^std::get<1>(key);
    }
  };
private:
  unordered_map<tuple<int,int>,vector<int>,tuple_hash> m_data;
};


class ZSegmentationEncoderRLXVector:public ZSegmentationEncoderRLX{
public:
  ZSegmentationEncoderRLXVector(){m_inited = false;}
  virtual ~ZSegmentationEncoderRLXVector(){
    map<int,int> m;
    for(vector<vector<int>>& yz: m_data){
      for(vector<int>& x:yz){
        m[x.size()/2]++;
      }
    }
    for(auto it = m.begin(); it != m.end(); ++it){
      std::cout<<it->first<<"->"<<it->second<<std::endl;
    }
    m_data.clear();
  }

public:
  virtual vector<int>& getSegment(int z, int y);
  virtual const vector<int>& getSegment(int z, int y)const;
  virtual bool hasSegment(int z, int y)const;
  virtual ZSegmentationEncoder* clone()const;
  virtual double memUsage()const;

private:
  vector<vector<vector<int>>> m_data;
  bool m_inited;
};


class ZSegmentationEncoderBitMap: public ZSegmentationEncoder{
public:
  ZSegmentationEncoderBitMap();
  virtual ~ZSegmentationEncoderBitMap(){m_data.clear();}

public:
  virtual void add(const ZIntPoint& point);

  virtual bool contains(const ZIntPoint& point)const;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder);

  virtual void labelStack(ZStack& stack, int value)const;

  virtual ZSegmentationEncoder* clone()const;

  virtual string type()const{return "BITMAP";}

  virtual double memUsage()const;

  //void clear();
  virtual void consume(const ZStack& stack);

  virtual ZIntCuboid getBoundBox()const{return ZIntCuboid(m_minx,m_miny,m_minz,m_maxx,m_maxy,m_maxz);}

private:
  inline void _maybe_update_bound_box(int z, int y, int start, int end);

protected:
  int m_minx, m_maxx, m_miny, m_maxy, m_minz, m_maxz;
  vector<int> m_data;
  bool m_inited;
};


class ZSegmentationEncoderRaw: public ZSegmentationEncoder{
public:
  ZSegmentationEncoderRaw();
  virtual ~ZSegmentationEncoderRaw(){}

public:
  virtual void add(const ZIntPoint& point);

  virtual bool contains(const ZIntPoint& point)const;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder);

  virtual void labelStack(ZStack& stack, int value)const;

  virtual ZSegmentationEncoder* clone()const;

  virtual string type()const{return "RAW";}

  virtual double memUsage()const;

  virtual void consume(const ZStack& stack);

  //void clear();

  virtual ZIntCuboid getBoundBox()const{return ZIntCuboid(m_minx,m_miny,m_minz,m_maxx,m_maxy,m_maxz);}

private:
  inline void _maybe_update_bound_box(int z, int y, int start, int end);

protected:
  int m_minx, m_maxx, m_miny, m_maxy, m_minz, m_maxz;
  std::shared_ptr<ZStack> m_data;
  bool m_inited;
};


class ZSegmentationEncoderScan: public ZSegmentationEncoder{
public:
  ZSegmentationEncoderScan(){m_data = std::make_shared<ZObject3dScan>();}
  virtual ~ZSegmentationEncoderScan(){}

public:
  virtual void add(const ZIntPoint& point);

  virtual bool contains(const ZIntPoint& point)const;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder);

  virtual void labelStack(ZStack& stack, int value)const;

  virtual ZSegmentationEncoder* clone()const;

  virtual string type()const{return "SCAN";}

  virtual double memUsage()const;

  //void clear();

  virtual ZIntCuboid getBoundBox()const;

protected:
  std::shared_ptr<ZObject3dScan> m_data;
};



class ZSegmentationEncoderOctTree: public ZSegmentationEncoder{
public:
  struct OctTree{
    OctTree(){for(int i=0; i<8; ++i){child[i]=nullptr;}}
    ~OctTree(){
       destroy();
    }

    void destroy(){
      for(int i = 0; i<8; ++i){
        if(child[i]){
          child[i]->destroy();
          delete child[i];
          child[i] = nullptr;
        }
      }
    }

    OctTree* child[8];
    ZIntCuboid box;
    bool leaf;
    bool contains(const ZIntPoint& p){
      if(!box.contains(p)){
        return false;
      }
      if(leaf){
        return true;
      }
      for(int i = 0; i < 8; ++i){
        if(child[i] && child[i]->box.contains(p)){
          return child[i]->contains(p);
        }
      }
      return false;
    }
    double memUsage(){
      double rv = 0;
      for(int i = 0; i < 8; ++i){
        if(child[i]){
          rv += child[i]->memUsage();
        }
      }
      rv += sizeof(OctTree);
      return rv;
    }
  };
public:
  ZSegmentationEncoderOctTree(){}
  virtual ~ZSegmentationEncoderOctTree(){}

public:
  virtual void add(const ZIntPoint& point);

  virtual bool contains(const ZIntPoint& point)const;

  virtual void unify(const ZSegmentationEncoder& segmentation_encoder){}

  virtual void labelStack(ZStack& stack, int value)const{}

  virtual ZSegmentationEncoder* clone()const{return nullptr;}

  virtual string type()const{return "OctTree";}

  virtual double memUsage()const{
    if(m_data){
      return m_data->memUsage();
    }
    return 0.0;
  }

  virtual void consume(const ZStack& stack);

  //void clear();

  virtual ZIntCuboid getBoundBox()const{return m_data->box;}

private:
  OctTree* _construct(const ZStack& stack, const ZIntCuboid& box);

protected:
  std::shared_ptr<OctTree> m_data;
};



class ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const = 0;
  virtual ~ZSegmentationEncoderFactory(){}
};


class ZSegmentationEncoderRLXTwoStageHashingFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderRLXTwoStageHashing();}
};


class ZSegmentationEncoderRLXOneStageHashingFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderRLXOneStageHashing();}
};


class ZSegmentationEncoderRLXVectorFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderRLXVector();}
};


class ZSegmentationEncoderBitMapFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderBitMap();}
};


class ZSegmentationEncoderRawFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderRaw();}
};


class ZSegmentationEncoderScanFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderScan();}
};


class ZSegmentationEncoderOctTreeFactory:public ZSegmentationEncoderFactory{
public:
  virtual ZSegmentationEncoder* create()const{return new ZSegmentationEncoderOctTree();}
};


#endif // ZSEGMENTATIONENCODER_H

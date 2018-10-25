#ifndef ZSEGMENTATIONSCAN_H
#define ZSEGMENTATIONSCAN_H

#include <vector>
#include "zintpoint.h"
#include "zintcuboid.h"

class ZStack;
class ZObject3dScan;

class ZSegmentationScan
{
public:
ZSegmentationScan();
~ZSegmentationScan(){};
//void init();

public:
void fromStack(ZStack* stack);
void fromObject3DScan(ZObject3dScan* obj);

ZObject3dScan* toObject3dScan();
ZStack* toStack();

void maskStack(ZStack* stack);
void labelStack(ZStack* stack, int v=1);

void setOffset(ZIntPoint offset){m_offset = offset;}
void setOffset(int x, int y, int z){m_offset = ZIntPoint(x,y,z);}
ZIntPoint getOffset()const{return m_offset;}

void unify(ZSegmentationScan* obj);
inline std::vector<int>& getStrip(int z, int y){return (z < m_sz || z > m_ez || y < m_sy || y > m_ey) ? m_empty_vec : getSlice(z)[y-m_sy];}
inline std::vector<std::vector<int>>& getSlice(int z){return (z < m_sz || z > m_ez) ? m_empty_vec_vec : m_data[z-m_sz];}
std::vector<std::vector<std::vector<int>>>& getData(){return m_data;}

public:
inline int minZ()const {return m_sz;}
inline int maxZ()const {return m_ez;}
inline int minY()const {return m_sy;}
inline int maxY()const {return m_ey;}
inline int minX()const {return m_sx;}
inline int maxX()const {return m_ex;}
inline ZIntCuboid getBoundBox()const{return ZIntCuboid(m_sx,m_sy,m_sz,m_ex,m_ey,m_ez);}
void translate(ZIntPoint offset);
void translate(int ofx, int ofy, int ofz);

private:
inline void addSegment(int z, int y, int sx, int ex);
ZIntCuboid getStackForegroundBoundBox(ZStack* stack);
std::vector<int> canonize(std::vector<int>&a, std::vector<int>& b);
void clearData();
void preprareData(int depth, int height);

private:
std::vector<std::vector<std::vector<int>>> m_data;
std::vector<int> m_empty_vec;
std::vector<std::vector<int>> m_empty_vec_vec;
int m_sz, m_ez;
int m_sy, m_ey;
int m_sx, m_ex;
ZIntPoint m_offset;
};

typedef std::vector<ZSegmentationScan*> ZSegmentationScanArray;

#endif // ZSEGMENTATIONSCAN_H

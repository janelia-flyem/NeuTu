#include "zsegmentationnodewrapper.h"
#include "zpainter.h"
#include "geometry/zcuboid.h"

bool ZSegmentationNodeWrapper::hit(double x, double y, double z){
  if(!isSelectable()){
    return false;
  }
  return m_tree->contains(m_id,x,y,z);
}

ZCuboid ZSegmentationNodeWrapper::getBoundBox() const
{
  ZIntCuboid box;

  vector<shared_ptr<ZSegmentationEncoder>> vec_encoders;

  for(auto id: m_tree->getLeavesIDs(m_id)){
    vec_encoders.push_back(m_tree->getEncoder(id));
  }

  for(auto encoder: vec_encoders){
    box.join(encoder->getBoundBox());
  }

  return ZCuboid::FromIntCuboid(box);
}


void ZSegmentationNodeWrapper::display(ZPainter &painter, int slice, EDisplayStyle option, neutu::EAxis sliceAxis) const{

  if (sliceAxis != m_sliceAxis || getColor().alpha() == 0) {
    return;
  }

  if(slice < 0){
    return;
  }

  if(!m_tree->hasID(m_id)){
    return;
  }

  QColor color = m_tree->getColor(m_id);
  QPen pen(color);

  if (isSelected()) {
      QColor color(Qt::white);
      color.setAlpha(120);
      pen.setColor(color);
  }
  painter.setPen(pen);

  int z = slice + int(painter.getZOffset());

  vector<shared_ptr<ZSegmentationEncoder>> vec_encoders;

  for(auto id: m_tree->getLeavesIDs(m_id)){
    vec_encoders.push_back(m_tree->getEncoder(id));
  }

  if(!vec_encoders.size()){
    return;
  }

  ZIntCuboid box;
  for(auto encoder: vec_encoders){
    box.join(encoder->getBoundBox());
  }

  if(box.getWidth() <= 0 || box.getHeight() <= 0 || box.getDepth() <= 0||
     z < box.getMinCorner().getZ() || z > box.getMaxCorner().getZ()){
    return;
  }

  option = displayStyle();

  for(auto p_encoder: vec_encoders){
    const ZSegmentationEncoder* encoder = p_encoder.get();
    if(option == ZStackObject::EDisplayStyle::BOUNDARY){
      ZIntCuboid box = encoder->getBoundBox();
      box.setMinZ(z);
      box.setMaxZ(z);
      shared_ptr<ZStack> stack = shared_ptr<ZStack>(new ZStack(GREY,box,1));
      uint8_t* p = stack->array8();

      int y0 = box.getMinCorner().getY();
      int y1 = box.getMaxCorner().getY();
      int x0 = box.getMinCorner().getX();

      int width = box.getWidth();
      int height = box.getHeight();

      for(int y = y0; y <= y1; ++y){
        const vector<int>& segs = encoder->getSegment(z,y);
        for(auto it = segs.begin(); it != segs.end();){
          int x1 = *it++;
          int x2 = *it++;
          int offset = (y - y0) * width + x1 - x0;
          for(int x = x1; x <= x2; ++x){
            p[offset++] = 1;
          }
        }
      }

      std::vector<QPoint> points;
      int nb[4] = {1,-1,width,-width};
      int max_off= stack->getVoxelNumber() - 1;
      for(int y = 0; y < height; ++y){
        int offset = y * width;
        for(int x = 0; x < width; ++x, ++offset){
          uint8 v = p[offset];
          if(v){
            if(x == 0 || y==0){
              points.push_back(QPoint(x+x0,y+y0));
            } else {
              for(int i = 0; i < 4; ++i){
                int off_nb = offset + nb[i];
                if(off_nb >= 0 && off_nb <= max_off && p[off_nb] != v){
                  points.push_back(QPoint(x+x0,y+y0));
                  break;
                }
              }
            }
          }
        }
      }
      //painter.drawPolyline(&points[0], points.size());
      painter.drawPoints(points);
    } else {
      box = encoder->getBoundBox();
      std::vector<QLine> lineArray;
      int y0 = box.getMinCorner().getY();
      int y1 = box.getMaxCorner().getY();
      int stride = 1;
      for(int y = y0; y <= y1; y += stride){
        const vector<int>& segments = encoder->getSegment(z,y);
        for(auto it = segments.begin(); it != segments.end(); ){
          int x1 = *it++;
          int x2 = *it++;
          lineArray.push_back(QLine(x1,y,x2,y));
        }
      }
      painter.drawLines(lineArray);
    }
  }
}


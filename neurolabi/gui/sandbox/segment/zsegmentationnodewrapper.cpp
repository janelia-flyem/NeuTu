#include"zsegmentationnodewrapper.h"
#include"zpainter.h"


bool ZSegmentationNodeWrapper::hit(double x, double y, double z){
  if(!isSelectable()){
    return false;
  }
  return m_tree->contains(m_id,x,y,z);
}


void ZSegmentationNodeWrapper::display(ZPainter &painter, int slice, EDisplayStyle /*option*/, neutube::EAxis sliceAxis) const{

  if (sliceAxis != m_sliceAxis || getColor().alpha() == 0) {
    return;
  }

  if(slice < 0){
    return;
  }

  if(!m_tree->hasID(m_id)){
    return;
  }

  QPen pen(m_color);

  if (isSelected()) {
      QColor color(Qt::white);
      color.setAlpha(164);
      pen.setColor(color);
  }
  painter.setPen(pen);

  std::vector<QLine> lineArray;

  int z = slice + int(painter.getZOffset());

  ZIntCuboid box = m_tree->getBoundBox(m_id);

  if(box.getWidth() <= 0 || box.getHeight() <= 0 || box.getDepth() <= 0||
     z < box.getFirstCorner().getZ() || z > box.getLastCorner().getZ()){
    return;
  }

  int min_y = box.getFirstCorner().getY();
  int max_y = box.getLastCorner().getY();
  int min_x = box.getFirstCorner().getX();
  int max_x = box.getLastCorner().getX();

  for(int y = min_y; y <= max_y; y += 6){
    bool first = true;
    int start_x, end_x;
    for(int x = min_x; x <= max_x + 1; ++x){
      if(m_tree->contains(m_id,x,y,z)){
        if(first){
          first = false;
          start_x = x;
          end_x = x + 1;
        } else {
          ++end_x;
        }
      } else {
        if(!first){
          first = true;
          --end_x;
          if(end_x - start_x > 1)
            lineArray.push_back(QLine(start_x,y,end_x,y));
        }
      }
    }
  }
  painter.drawLines(lineArray);
}

